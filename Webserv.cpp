/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:29:06 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/12/30 15:04:30 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Node.hpp"

Webserv::Webserv(ConfigParser* parser) : m_keepalive_timeout(KEEPALIVE_TIMEOUT)
{
	std::vector<const Node*> clientBufferSize;
	std::vector<const Node*> KeepaliveTimeout;
	std::ostringstream oss;
	QueryListener* ql;
	s_server server;

	char cwd[1024];
	getcwd(cwd, sizeof(cwd));      // lire le CWD
	chdir("/chemin/vers/binaire"); // changer le CWD
	
	m_client_buffers_size[0] = HEADER_BUFFER_SIZE;
	m_client_buffers_size[1] = BODY_BUFFER_SIZE;

	if (parser != NULL)
	{
		//define global variables server
		clientBufferSize = parser->getDirectives("client_body_buffer_size");
		for (std::vector<const Node*>::const_iterator it = clientBufferSize.begin(); it != clientBufferSize.end(); ++it)
			static_cast<const NodeDirective*>(*it)->getClientSize(m_client_buffers_size[1]);
		clientBufferSize = parser->getDirectives("client_header_buffer_size");
		for (std::vector<const Node*>::const_iterator it = clientBufferSize.begin(); it != clientBufferSize.end(); ++it)
			static_cast<const NodeDirective*>(*it)->getClientSize(m_client_buffers_size[0]);
		KeepaliveTimeout = parser->getDirectives("keepalive_timeout");
		for (std::vector<const Node*>::const_iterator it = KeepaliveTimeout.begin(); it != KeepaliveTimeout.end(); ++it)
			static_cast<const NodeDirective*>(*it)->getClientsTimeout(m_keepalive_timeout);
		//build servers structures instances
		m_servers = createServers(parser);
	}
	else
	{
		//build a default server
		server.ports.push_back(80);
		server.hosts.push_back("0.0.0.0");
		server.root = "/html";
		m_servers.push_back(server);
		ql = createListener(server.ports[0], server.hosts[0]);
		if (ql)
			m_listeners.push_back(ql);
	}
	oss << "buffer Header_size: " << m_client_buffers_size[0];
	logOutMessage(oss);
	oss << "buffer Body_size: " << m_client_buffers_size[1];
	logOutMessage(oss);
	oss << "Keepalive timeout: " << m_keepalive_timeout;
	logOutMessage(oss);
}

Webserv::~Webserv()
{
	cleanWebserv();
}

void Webserv::startListening(void (*onResponse)(std::string&, CGI*, ParserHttpRequest&, s_server&))
{
	pollfd pfd;
	rlimit limit;
	s_query* q;
	int fd;
	std::ostringstream oss;

	//check system queue size
	if (getrlimit(RLIMIT_NOFILE, &limit) == -1)
	{
		cleanWebserv();
		throw std::runtime_error(std::strerror(errno));	
	}
	for (std::vector<const QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		pfd.fd = (*it)->getListenFD();
		pfd.events = POLLIN;
		pfd.revents = 0;
		m_fds.push_back(pfd);
		m_fdType.push_back(SOCKET);
	}
	oss << "Listening...";
	logOutMessage(oss);
	while (g_listening)
	{
		if (poll(reinterpret_cast<pollfd*>(m_fds.data()), m_fds.size(), 500) < 0)
		{
			g_listening = false;
			break ;
		}
		for (size_t i = 0; i < m_fds.size(); ++i)
		{
			fd = m_fds[i].fd;
			if (m_fdType[i] == SOCKET && (m_fds[i].revents & POLLIN))
			{
				addClient(fd);
				break ;
			}
			else if (m_fdType[i] == PIPE && getCgiQuery(fd, q))
			{
				if (m_fds[i].revents & POLLIN)
				{
					if (q->cgi->readCGI() <= 0)
					{
						onResponse(q->formatedResponse, q->cgi, *q->httpParser, getRightServer(q));
						delete (q->cgi);
						q->cgi = NULL;
						break ;
					}
				}
				else if (m_fds[i].revents & POLLOUT)
				{
					for (size_t i = 0; i < q->bodyChunks.size(); ++i)
						if (q->cgi->writeCGI(q->bodyChunks[i]) < 0)
						{
							delete (q->cgi);
							q->cgi = NULL;
							break ;
						}
				}
			}
			else if (m_fdType[i] == ACCEPT)
			{
				if (m_fds[i].revents & POLLOUT)
					if (sendQuery(fd) == -1)
					{
						m_fds[i].events &= ~POLLOUT;
						oss << "Deconnected client fd:" << m_fds[i].fd;
						logOutMessage(oss);
						destroyClient(fd);
						break ;
					}
				if (m_fds[i].revents & POLLIN)
				{
					if (readQuery(fd, onResponse) == -2)
					{
						g_listening = false;
						break ;
					}
				}
				if (clientNeedsAnswer(fd))
					m_fds[i].events |= POLLOUT;
				else if (m_fds[i].events & POLLOUT)
				{
					m_fds[i].events &= ~POLLOUT;
					oss << "Deconnected client fd:" << m_fds[i].fd;
					logOutMessage(oss);
					destroyClient(fd);
					break ;
				}
				if (!keepAlive(fd, m_keepalive_timeout))
				{
					oss << "Deconnected client fd:" << m_fds[i].fd;
					logOutMessage(oss);
					destroyClient(fd);
					break ;
				}
			}
		}
	}
	cleanWebserv();
	oss << "Stop listening";
	logOutMessage(oss);
}

void Webserv::addClient(int fd)
{
	static sockaddr_in serverAddress;
	static socklen_t serverlen;
	static s_query query;
	static pollfd pfd;
	std::ostringstream oss;
	size_t j;
	char ip[INET_ADDRSTRLEN];

	for (std::vector<const QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{	
		serverAddress = (*it)->getServerAddress();
		serverlen = sizeof(serverAddress);
		pfd.fd = accept(fd, (struct sockaddr*)&serverAddress, &serverlen);
		if (pfd.fd < 0) 
			continue;
		if (fcntl(pfd.fd, F_SETFL, O_NONBLOCK) == -1)
		{
			oss << std::strerror(errno);
			logErrMessage(oss);
		}
		pfd.events = POLLIN;
		pfd.revents = 0;
		for (j = 0; j < m_fds.size(); ++j)
			if (m_fds[j].fd == pfd.fd)
				break;
		if (j == m_fds.size())
		{
			m_fds.push_back(pfd);
			m_fdType.push_back(ACCEPT);
			getsockname(fd, (struct sockaddr*)&serverAddress, &serverlen);
			query.fd = pfd.fd;
			query.lifeTime = std::time(NULL);
			query.port = ntohs(serverAddress.sin_port);
			inet_ntop(AF_INET, &(serverAddress.sin_addr), ip, serverlen);
			query.host = ip;
			m_clients.push_back(query);
			oss << "New client connected: fd:" << pfd.fd << ", port:"<< ntohs(serverAddress.sin_port);
			logOutMessage(oss);
		}
		break;
	}
}

int Webserv::readQuery(int fd, void (*onResponse)(std::string&, CGI*, ParserHttpRequest&, s_server&))
{
	std::ostringstream oss;
	char *buffers;
	bool bBody;
	s_query *client;
	bool bDelete;
	ssize_t n;
	int ret;
	
	n = 0;
	bDelete = true;
	bBody = false;
	buffers = NULL;
	if (getClient(fd, client))
	{
		do
		{
			client->lifeTime = std::time(NULL);
			buffers = new (std::nothrow) char[m_client_buffers_size[bBody]];
			if (buffers == NULL)
			{
				cleanWebserv();
				throw std::bad_alloc();
			}
			if ((n = read(client->fd, buffers, m_client_buffers_size[bBody] - 1)) > 0)
			{
				buffers[n] = '\0';
				ret = tcpStream(buffers, n, client, onResponse, bDelete);
				if(ret < 0)
				{
					if (bDelete)
					{
						delete [](buffers);
						buffers = NULL;
					}
					return (-2);
				}
				client->bodySize ? bBody = true : bBody = false;
				if (bDelete)
				{
					delete [](buffers);
					buffers = NULL;
				}
			}
		} while(n > 0);
		if (n == -1)
		{
			oss << "client fd:" << client->fd << ", " << std::strerror(errno);
			logErrMessage(oss);
		}
		delete [](buffers);
	}
	return (n);
}

int Webserv::sendQuery(int fd)
{
	ssize_t n;
	s_query *client;
	std::ostringstream oss;

	n = 0;
	for (std::vector<s_query>::iterator it = m_queries.begin(); it != m_queries.end(); ++it)
	{
		if ((*it).fd == fd)
		{
			do
			{
				if(getClient(fd, client))
					client->lifeTime = std::time(NULL);
				n = send((*it).fd, (*it).formatedResponse.data() + (*it).byteSent
					, (*it).formatedResponse.size() - (*it).byteSent, 0);
				if (n > 0)
				{
					(*it).byteSent += n;
					if ((*it).byteSent == (*it).formatedResponse.size())
					{
						(*it).formatedResponse.clear();
						break ;
					}
				}
				else if (n == -1)
				{
					oss << "client fd:" << (*it).fd << ", " << std::strerror(errno);
					logErrMessage(oss);
				}
				else if (n == 0)
				{
					oss << "client fd:" << (*it).fd << ", send 0 byte";
					logOutMessage(oss);
				}
			}
			while (n > 0);
		}
	}
	return (n);
}

bool Webserv::clientNeedsAnswer(int fd) const
{
	for (std::vector<s_query>::const_iterator it = m_queries.begin(); it != m_queries.end(); ++it)
	{
		if ((*it).fd == fd && !(*it).formatedResponse.empty())
			return (true);
	}
	return (false);
}

std::vector<s_server> Webserv::createServers(const ConfigParser* parser)
{
	std::vector<const Node*> _servers;
	std::vector<const Node*> _listens;
	std::vector<const Node*> _roots;
	std::vector<const Node*> _alias;
	std::vector<const Node*> _server_names;
	std::vector<const Node*> _location;
	std::vector<const Node*> _limit_except;
	std::vector<const Node*> _max_body_size;
	std::vector<const Node*> _cgi_pass;
	std::vector<const Node*> _index;
	std::vector<s_server> servers;
	std::vector<std::string> args;
	std::ostringstream oss;
	std::string _host;
	s_location loc;
	QueryListener* ql;
	uint16_t _port;
	s_server _server;

	if (parser == NULL)
	{
		oss << "No config file.";
		logOutMessage(oss);
	}
	_roots = parser->getDirectives("root");
	_max_body_size = parser->getDirectives("client_max_body_size");
	_servers = parser->getDirectives("server");
	for (std::vector<const Node*>::const_iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		_server.ports.clear();
		_server.hosts.clear();
		_server.server_names.clear();
		_server.locations.clear();
		_port = 80;
		_host = "0.0.0.0";

		args = getDiretiveValue(*it, _roots);
		if (args.empty())
			_server.root = "/html";
		else
			_server.root = args[0];
		args = getDiretiveValue(*it, _max_body_size);
		if (args.empty())
			_server.max_body_size = "8192";
		else
			_server.max_body_size = args[0];
		_listens = parser->getDirectives("listen", static_cast<const NodeBlock*>(*it));
		if (_listens.size() == 0)
		{
			ql = createListener(_port, _host);
			if (ql)
				m_listeners.push_back(ql);
		}
		else
			for (std::vector<const Node*>::const_iterator it1 = _listens.begin(); it1 != _listens.end(); ++it1)
			{
				_port = 80;
				_host = "0.0.0.0";
				if (!static_cast<const NodeDirective*>(*it1)->getListenHostPort(_port, _host))
				{
					_server.ports.push_back(_port);
					_server.hosts.push_back(_host);
					ql = createListener(_port, _host);
					if (ql)
						m_listeners.push_back(ql);
				}
			}
		_server_names = parser->getDirectives("server_name", static_cast<const NodeBlock*>(*it));
		for (std::vector<const Node*>::const_iterator it1 = _server_names.begin(); it1 != _server_names.end(); ++it1)
		{
			args = (*it1)->getArgs();
			for (std::vector<std::string>::iterator it1 = args.begin(); it1 != args.end(); ++it1)
				_server.server_names.push_back(*it1);
		}
		_location = parser->getDirectives("location", static_cast<const NodeBlock*>(*it));
		for (std::vector<const Node*>::const_iterator it1 = _location.begin(); it1 != _location.end(); ++it1)
		{
			loc.concatOrReplace = (*it1)->getArgs()[0];
			loc.is_cgi = false;
			loc.type = LOCATION_NONE;
			loc.by = "none";
			loc.max_body_size = "not define";
			loc.cgi_pass = "none";
			loc.httpMethodsAllowed.clear();
			for (std::vector<const Node*>::const_iterator it2 = _roots.begin(); it2 != _roots.end(); ++it2)
				if ((*it2)->getParent() == *it1)
				{
					loc.type = LOCATION_ROOT;
					loc.by = (*it2)->getArgs()[0];
				}
			_alias = parser->getDirectives("alias", static_cast<const NodeBlock*>(*it1));
			if (_alias.size())
			{
				loc.type = LOCATION_ALIAS;
				loc.by = _alias[0]->getArgs()[0];
			}
			for (std::vector<const Node*>::const_iterator it2 = _max_body_size.begin(); it2 != _max_body_size.end(); ++it2)
				if ((*it2)->getParent() == *it1)
					loc.max_body_size = (*it2)->getArgs()[0];
			_cgi_pass = parser->getDirectives("cgi_pass", static_cast<const NodeBlock*>(*it1));
			if (_cgi_pass.size())
			{
				loc.is_cgi = true;
				if (_cgi_pass[0]->getArgs().size())
					loc.cgi_pass = _cgi_pass[0]->getArgs()[0];
				else
					loc.cgi_pass = "";
			}
			_index = parser->getDirectives("index", static_cast<const NodeBlock*>(*it1));
			if (_index.size())
			{
				if (_index[0]->getArgs().size())
					loc.index = _index[0]->getArgs()[0];
			}
			_limit_except = parser->getDirectives("limit_except", static_cast<const NodeBlock*>(*it1));
			if (_limit_except.size())
			{
				for (size_t i = 0; i < _limit_except[0]->getArgs().size(); ++i)
				{
					HttpMethod method;
					if (static_cast<const NodeDirective*>(_limit_except[0])->getHttpMethod(i, method) == 0)
						loc.httpMethodsAllowed.push_back(method);
				}
			}
			_server.locations.push_back(loc);
		}
		servers.push_back(_server);
	}
	return (servers);
}

const std::vector<std::string> Webserv::getDiretiveValue(const Node* server, const std::vector<const Node*> list) const
{
	std::vector<std::string> args;
	int currentDeep = 0;

	for (std::vector<const Node*>::const_iterator directive = list.begin(); directive != list.end(); ++directive)
	{
		if ((*directive)->getDeep() == server->getDeep())
		{
			for (Node* ptr = (*directive)->getParent(); ptr != NULL; ptr = ptr->getParent())
				if (ptr == server && currentDeep <= (*directive)->getDeep())
				{
					currentDeep = (*directive)->getDeep();
					args = (*directive)->getArgs();
				}
		}
	}
	return (args);
}

void Webserv::cleanWebserv()
{
	stopListening();
	for (std::vector<const QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
		delete (*it);
	m_listeners.clear();
	for (size_t i = 0; i < m_clients.size(); ++i)
		destroyClient(m_clients[i].fd);
	m_fds.clear();
	m_fdType.clear();
}

void Webserv::stopListening()
{
	g_listening = false;
}

QueryListener* Webserv::createListener(u_int16_t port, const std::string& host)
{
	try
	{
		return (new QueryListener(port, host));
	}
	catch(const std::exception& e)
	{
		return (NULL);
	}	
}