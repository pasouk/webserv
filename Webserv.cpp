/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:29:06 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/10/08 14:43:41 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Node.hpp"

Webserv::Webserv(ConfigParser* parser) : m_parser(parser), m_keepalive_timeout(KEEPALIVE_TIMEOUT)
{
	std::vector<const Node*> clientBufferSize;
	std::vector<const Node*> KeepaliveTimeout;

	m_client_buffers_size[0] = HEADER_BUFFER_SIZE;
	m_client_buffers_size[1] = BODY_BUFFER_SIZE;

	if (parser == NULL)
		throw std::runtime_error("No configuration.");
	
	//build servers structures instances
	m_servers = createServers();

	//define global variables server
	clientBufferSize = parser->getDirectives("client_body_buffer_size");
	for (std::vector<const Node*>::const_iterator it = clientBufferSize.begin(); it != clientBufferSize.end(); ++it)
		static_cast<const NodeDirective*>(*it)->getClientBufferSize(m_client_buffers_size[1]);
	clientBufferSize = parser->getDirectives("client_header_buffer_size");
	for (std::vector<const Node*>::const_iterator it = clientBufferSize.begin(); it != clientBufferSize.end(); ++it)
		static_cast<const NodeDirective*>(*it)->getClientBufferSize(m_client_buffers_size[0]);
	KeepaliveTimeout = parser->getDirectives("keepalive_timeout");
	for (std::vector<const Node*>::const_iterator it = KeepaliveTimeout.begin(); it != KeepaliveTimeout.end(); ++it)
		static_cast<const NodeDirective*>(*it)->getClientsTimeout(m_keepalive_timeout);
	std::cout << "buffer Header_size: " << m_client_buffers_size[0] << std::endl;
	std::cout << "buffer Body_size: " << m_client_buffers_size[1] << std::endl;
	std::cout << "Keepalive timeout: " << m_keepalive_timeout << std::endl;
}

Webserv::~Webserv()
{
	cleanWebserv();
}

void Webserv::startListening(void (*onResponse)(std::string&, ParserHttpRequest&, const server&))
{
	pollfd fd;
	static rlimit limit;

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
		m_isClient.push_back(false);
	}
	std::cout << "Listening...\n";
	while (g_listening)
	{
		if (poll(reinterpret_cast<pollfd*>(m_fds.data()), m_fds.size(), 500) < 0)
		{
			g_listening = false;
			break;
		}
		for (size_t i = 0; i < m_fds.size(); ++i)
		{
			if (!m_isClient[i] && (m_fds[i].revents & POLLIN))
				addClient(i);
			if (m_isClient[i])
			{
				if (m_fds[i].revents & POLLIN)
					readQuery(i, onResponse);
				if (clientNeedsAnswer(i))
					m_fds[i].events |= POLLOUT;
				else
				{
					m_fds[i].events &= ~POLLOUT;
					destroyClientQueries(i);
				}
				if (!keepAlive(i, m_keepalive_timeout))
				{
					std::cout << "Deconnected client fd:" << m_fds[i].fd << std::endl;
					destroyClientQueries(i);
					destroyClient(i);
				}
				if (m_fds[i].revents & POLLOUT)
					sendQuery(i);
			}
		}
	}
	cleanWebserv();
	std::cout << "Stop listening\n";
}

void Webserv::addClient(size_t i)
{
	static sockaddr_in serverAddress;
	static socklen_t serverlen;
	static query query;
	static pollfd fd;
	size_t j;
	char ip[INET_ADDRSTRLEN];

	for (std::vector<const QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{					
		serverAddress = (*it)->getServerAddress();
		serverlen = sizeof(serverAddress);
		fd.fd = accept(m_fds[i].fd, (struct sockaddr*)&serverAddress, &serverlen);
		if (fd.fd < 0) 
			continue;
		fd.events |= POLLIN;
		fd.revents = 0;
		for (j = 0; j < m_fds.size(); ++j)
			if (m_fds[j].fd == fd.fd)
				break;
		if (j == m_fds.size())
		{
			m_fds.push_back(fd);
			m_isClient.push_back(true);
			getsockname(m_fds[i].fd, (struct sockaddr*)&serverAddress, &serverlen);
			query.fd = fd.fd;
			query.lifeTime = std::time(NULL);
			query.port = ntohs(serverAddress.sin_port);
			inet_ntop(AF_INET, &(serverAddress.sin_addr), ip, serverlen);
			query.host = ip;
			m_clients.push_back(query);
			std::cout << "New client connected: fd:" << fd.fd
				<< ", port:"<< ntohs(serverAddress.sin_port) << std::endl;
		}
		break;
	}
}

void Webserv::readQuery(size_t i, void (*onResponse)(std::string&, ParserHttpRequest&, const server&))
{
	static sockaddr_in serverAddress;
	static socklen_t serverlen;
	static char *buffers;
	static bool bBody;
	bool bDelete;
	query client;
	ssize_t n;
	
	bDelete = true;
	buffers = new (std::nothrow) char[m_client_buffers_size[bBody]];
	if (buffers == NULL)
	{
		cleanWebserv();
		throw std::bad_alloc();
	}
	getsockname(m_fds[i].fd, (struct sockaddr*)&serverAddress, &serverlen);
	if(getClient(i, client))
		client.lifeTime = std::time(NULL);
	n = read(m_fds[i].fd, buffers, m_client_buffers_size[bBody] - 1);
	if (n > 0)
	{
		buffers[n] = '\0';
		for (std::vector<query>::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
			if (m_fds[i].fd == (*it).fd)
			{
				bDelete = tcpStream(buffers, n, it, onResponse);
				(*it).bodySize ? bBody = true : bBody = false;
				break;
			}
	}
	else if (n == -1)
		std::cerr << "read: " << std::strerror(errno) << std::endl;
	if (bDelete)
		delete [](buffers);
}

void Webserv::sendQuery(size_t i)
{
	ssize_t n;
	query client;

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
					std::cerr << "send: " << std::strerror(errno) << std::endl;
					break ;
				}
				else
				{ 
					cleanWebserv();
					throw std::runtime_error(std::strerror(errno));
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

std::vector<server> Webserv::createServers()
{
	std::vector<const Node*> _servers;
	std::vector<const Node*> _listens;
	std::vector<const Node*> _roots;
	std::vector<const Node*> _alias;
	std::vector<const Node*> _server_names;
	std::vector<const Node*> _location;
	std::vector<const Node*> _limit_except;
	std::vector<server> servers;
	std::vector<std::string> args;
	std::string _host;
	location loc;
	QueryListener* ql;
	uint16_t _port;
	server _server;

	_roots = m_parser->getDirectives("root");
	_servers = m_parser->getDirectives("server");
	for (std::vector<const Node*>::const_iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		_server.ports.clear();
		_server.hosts.clear();
		_server.server_names.clear();
		_server.locations.clear();
		_port = 80;
		_host = "0.0.0.0";

		args = getDeeperValue(*it, _roots);
		if (args.empty())
			_server.root = "/html";
		else
			_server.root = args[0];
		_listens = m_parser->getDirectives("listen", static_cast<const NodeBlock*>(*it));
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
		_server_names = m_parser->getDirectives("server_name", static_cast<const NodeBlock*>(*it));
		for (std::vector<const Node*>::const_iterator it1 = _server_names.begin(); it1 != _server_names.end(); ++it1)
		{
			args = (*it1)->getArgs();
			for (std::vector<std::string>::iterator it1 = args.begin(); it1 != args.end(); ++it1)
				_server.server_names.push_back(*it1);
		}
		_location = m_parser->getDirectives("location", static_cast<const NodeBlock*>(*it));
		for (std::vector<const Node*>::const_iterator it1 = _location.begin(); it1 != _location.end(); ++it1)
		{
			for (std::vector<const Node*>::const_iterator it2 = _roots.begin(); it2 != _roots.end(); ++it2)
				if ((*it2)->getParent() == *it1)
				{
					loc.type = ROOT;
					loc.path = (*it2)->getArgs()[0];
					_server.locations[(*it1)->getArgs()[0]] = loc;
				}
			_alias = m_parser->getDirectives("alias", static_cast<const NodeBlock*>(*it1));
			if (_alias.size())
			{
				loc.type = ALIAS;
				loc.path = _alias[0]->getArgs()[0];
				_server.locations[(*it1)->getArgs()[0]] = loc;	
			}
			_limit_except = m_parser->getDirectives("limit_except", static_cast<const NodeBlock*>(*it1));
			if (_limit_except.size())
				for (size_t i = 0; i < _limit_except[0]->getArgs().size(); ++i)
				{
					HttpMethod method;
					if (static_cast<const NodeDirective*>(_limit_except[0])->getHttpMethod(i, method))
						_server.httpMethodsAllowed.push_back(method);
				}
		}
		servers.push_back(_server);
	}
	return (servers);
}

const std::vector<std::string> Webserv::getDeeperValue(const Node* server, const std::vector<const Node*> list) const
{
	std::vector<std::string> args;
	int currentDeep = 0;

	for (std::vector<const Node*>::const_iterator directive = list.begin(); directive != list.end(); ++directive)
	{
		if ((*directive)->getDeep() >= server->getDeep())
		{
			for (Node* ptr = (*directive)->getParent(); ptr != NULL; ptr = ptr->getParent())
				if (ptr == server && currentDeep <= (*directive)->getDeep())
				{
					currentDeep = (*directive)->getDeep();
					args = (*directive)->getArgs();
				}
		}
		else
		{
			if (currentDeep < (*directive)->getDeep())
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
		if (m_isClient[i])
		{
			destroyClientQueries(i);
			close (m_fds[i].fd);
		}
	m_fds.clear();
	m_isClient.clear();
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