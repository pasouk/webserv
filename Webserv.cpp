/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:29:06 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/11/05 07:15:48 by fabrice          ###   ########.fr       */
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
	server server;

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

void Webserv::startListening(void (*onResponse)(std::string&, ParserHttpRequest&, server&))
{
	static pollfd fd;
	static rlimit limit;
	query* q;
	std::ostringstream oss;
	std::string cgiResponse;

	//check system queue size
	if (getrlimit(RLIMIT_NOFILE, &limit) == -1)
	{
		cleanWebserv();
		throw std::runtime_error(std::strerror(errno));	
	}
	for (std::vector<const QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		fd.fd = (*it)->getListenFD();
		fd.events = POLLIN;
		fd.revents = 0;
		m_fds.push_back(fd);
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
			if (m_fdType[i] == SOCKET && (m_fds[i].revents & POLLIN))
				addClient(i);
			else if (m_fdType[i] == PIPE && getCgiQuery(m_fds[i].fd, q))
			{
				if (m_fds[i].revents & POLLIN)
				{
					if (q->cgi->readCGI(cgiResponse) <= 0)
					{
						const pollfd *fds = q->cgi->getPollfd();
						pollfd (&arr)[2] = *reinterpret_cast<pollfd (*)[2]>(const_cast<pollfd *>(fds));
						removePipeFromPoll(arr);
						releaseQuery(q);
					}
					std::cout << cgiResponse << std::endl;
				}
				else if (m_fds[i].revents & POLLOUT)
				{
					for (size_t i = 0; i < q->bodyChunks.size(); ++i)
						if (q->cgi->writeCGI(q->bodyChunks[i]) <= 0)
						{
							const pollfd *fds = q->cgi->getPollfd();
							pollfd (&arr)[2] = *reinterpret_cast<pollfd (*)[2]>(const_cast<pollfd *>(fds));
							removePipeFromPoll(arr);
							releaseQuery(q);
						}
				}
			}
			else if (m_fdType[i] == ACCEPT)
			{
				if (m_fds[i].revents & POLLIN)
					if (readQuery(i, onResponse) == 0)
					{
						oss << "Deconnected client fd:" << m_fds[i].fd;
						logOutMessage(oss);
						destroyClient(i);
					}
				if (clientNeedsAnswer(i))
					m_fds[i].events |= POLLOUT;
				else
				{				
					m_fds[i].events &= ~POLLOUT;
					releaseQueries(i);
				}
				if (!keepAlive(i, m_keepalive_timeout))
				{
					oss << "Deconnected client fd:" << m_fds[i].fd;
					logOutMessage(oss);
					destroyClient(i);
				}
				if (m_fds[i].revents & POLLOUT)
					sendQuery(i);
			}
		}
	}
	cleanWebserv();
	oss << "Stop listening";
	logOutMessage(oss);
}

void Webserv::addClient(size_t i)
{
	static sockaddr_in serverAddress;
	static socklen_t serverlen;
	static query query;
	static pollfd fd;
	std::ostringstream oss;
	size_t j;
	char ip[INET_ADDRSTRLEN];

	for (std::vector<const QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{					
		serverAddress = (*it)->getServerAddress();
		serverlen = sizeof(serverAddress);
		fd.fd = accept(m_fds[i].fd, (struct sockaddr*)&serverAddress, &serverlen);
		if (fd.fd < 0) 
			continue;
		if (fcntl(fd.fd, F_SETFL, O_NONBLOCK) == -1)
		{
			oss << std::strerror(errno);
			logErrMessage(oss);
		}
		fd.events |= POLLIN;
		fd.revents = 0;
		for (j = 0; j < m_fds.size(); ++j)
			if (m_fds[j].fd == fd.fd)
				break;
		if (j == m_fds.size())
		{
			m_fds.push_back(fd);
			m_fdType.push_back(ACCEPT);
			getsockname(m_fds[i].fd, (struct sockaddr*)&serverAddress, &serverlen);
			query.fd = fd.fd;
			query.lifeTime = std::time(NULL);
			query.port = ntohs(serverAddress.sin_port);
			inet_ntop(AF_INET, &(serverAddress.sin_addr), ip, serverlen);
			query.host = ip;
			m_clients.push_back(query);
			oss << "New client connected: fd:" << fd.fd << ", port:"<< ntohs(serverAddress.sin_port);
			logOutMessage(oss);
		}
		break;
	}
}

int Webserv::readQuery(size_t i, void (*onResponse)(std::string&, ParserHttpRequest&, server&))
{
	static char *buffers;
	static bool bBody;
	std::ostringstream oss;
	bool bDelete;
	ssize_t n;
	
	n = 0;
	bDelete = true;
	for (std::vector<query>::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
		if (m_fds[i].fd == (*it).fd)
		{
			(*it).lifeTime = std::time(NULL);
			do
			{
				buffers = new (std::nothrow) char[m_client_buffers_size[bBody]];
				if (buffers == NULL)
				{
					cleanWebserv();
					throw std::bad_alloc();
				}
				if ((n = read((*it).fd, buffers, m_client_buffers_size[bBody] - 1)) > 0)
				{
					buffers[n] = '\0';
					if(tcpStream(buffers, n, it, onResponse, bDelete))
						return (delete [](buffers), -1);
					(*it).bodySize ? bBody = true : bBody = false;
					if (bDelete)
						delete [](buffers);
				}
			} while(n > 0);
			if (n == -1)
			{
				oss << "read: " << std::strerror(errno);
				logOutMessage(oss);
			}
			break ;
		}
	return (delete [](buffers), n);
}

void Webserv::sendQuery(size_t i)
{
	ssize_t n;
	query client;
	std::ostringstream oss;

	for (std::vector<query>::iterator it = m_queries.begin(); it != m_queries.end(); ++it)
	{
		if ((*it).fd == m_fds[i].fd)
		{
			while ((*it).byteSent < (*it).formatedResponse.size())
			{
				if(getClient(i, client))
					client.lifeTime = std::time(NULL);
				n = send((*it).fd, (*it).formatedResponse.data() + (*it).byteSent
					, (*it).formatedResponse.size() - (*it).byteSent, 0);
				if (n > 0)
					(*it).byteSent += n;
				else if (n == -1)
				{
					oss << "send: " << std::strerror(errno);
					logOutMessage(oss);
					break ;
				}
				else
				{
					destroyClient(i);
					break ;
				}
			}
			if ((*it).byteSent == (*it).formatedResponse.size())
			{
				(*it).formatedResponse.clear();
				break;
			}
		}
	}
}

bool Webserv::clientNeedsAnswer(size_t i) const
{
	for (std::vector<query>::const_iterator it = m_queries.begin(); it != m_queries.end(); ++it)
		if ((*it).fd == m_fds[i].fd && !(*it).formatedResponse.empty())
			return (true);
	return (false);
}

std::vector<server> Webserv::createServers(const ConfigParser* parser)
{
	std::vector<const Node*> _servers;
	std::vector<const Node*> _listens;
	std::vector<const Node*> _roots;
	std::vector<const Node*> _alias;
	std::vector<const Node*> _server_names;
	std::vector<const Node*> _location;
	std::vector<const Node*> _limit_except;
	std::vector<const Node*> _max_body_size;
	std::vector<server> servers;
	std::vector<std::string> args;
	std::ostringstream oss;
	std::string _host;
	location loc;
	QueryListener* ql;
	uint16_t _port;
	server _server;

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
			loc.type = NONE;
			loc.concatOrReplace = "none";
			loc.by = "none";
			loc.max_body_size = "not define";
			loc.httpMethodsAllowed.clear();
			loc.concatOrReplace = (*it1)->getArgs()[0];
			for (std::vector<const Node*>::const_iterator it2 = _roots.begin(); it2 != _roots.end(); ++it2)
				if ((*it2)->getParent() == *it1)
				{
					loc.type = ROOT;
					loc.by = (*it2)->getArgs()[0];
				}
			_alias = parser->getDirectives("alias", static_cast<const NodeBlock*>(*it1));
			if (_alias.size())
			{
				loc.type = ALIAS;
				loc.by = _alias[0]->getArgs()[0];
			}
			for (std::vector<const Node*>::const_iterator it2 = _max_body_size.begin(); it2 != _max_body_size.end(); ++it2)
				if ((*it2)->getParent() == *it1)
					loc.max_body_size = (*it2)->getArgs()[0];
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
	for (size_t i = 0; i < m_fds.size(); ++i)
		if (m_fdType[i])
		{
			destroyClient(i);
			close (m_fds[i].fd);
		}
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