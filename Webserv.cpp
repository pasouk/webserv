/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:29:06 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/29 13:00:03 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Node.hpp"

Webserv::Webserv(ConfigParser* parser) : m_parser(parser), m_keepalive_timeout(KEEPALIVE_TIMEOUT)
{
	QueryListener* ql;
	std::vector<std::string> args;
	std::vector<const Node*> servers;
	std::vector<const Node*> clientBufferSize;
	std::vector<const Node*> KeepaliveTimeout;
	std::vector<const Node*> listens;
	std::vector<Node*>::const_iterator it2;
	uint16_t port;
	std::string host;

	m_client_buffers_size[0] = HEADER_BUFFER_SIZE;
	m_client_buffers_size[1] = BODY_BUFFER_SIZE;

	if (parser == NULL)
		throw std::runtime_error("No configuration.");
	
	//build servers structures
	m_servers = createServers();

	//buld listeners
	servers = parser->getDirectives("server");
	for (std::vector<const Node*>::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		port = 80;
		host = "0.0.0.0";
		listens = parser->getDirectives("listen", static_cast<const NodeBlock*>(*it));
		if (listens.size() == 0)
		{
			ql = createListener(port, host);
			if (ql)
				m_listeners.push_back(ql);
		}
		else
		{
			for (std::vector<const Node*>::const_iterator it = listens.begin(); it != listens.end(); ++it)
			{
				port = 80;
				host = "0.0.0.0";
				if (!static_cast<const NodeDirective*>(*it)->getListenHostPort(port, host))
				{
					ql = createListener(port, host);
					if (ql)
						m_listeners.push_back(ql);
				}
			}
		}
	}

	//define global varaibles server
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

void Webserv::startListening(void (*onQuery)(query&, std::vector<server>&, Webserv*))
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
					readQuery(i, onQuery);
				if (needAResponse(i))
					m_fds[i].events |= POLLOUT;
				else
				{
					m_fds[i].events &= ~POLLOUT;
					destroyClientQueries(i);
				}
				if (m_fds[i].revents & POLLOUT)
					sendQuery(i);
				if (!keepAlive(i, m_keepalive_timeout))
				{			
					std::cout << "Deconnected client fd:" << m_fds[i].fd << std::endl;
					destroyClientQueries(i);
					destroyClient(i);
				}
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

void Webserv::readQuery(size_t i, void (*onQuery)(query&, std::vector<server>&, Webserv*))
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
				bDelete = tcpStream(buffers, n, it, onQuery);
				(*it).bodySize ? bBody = true : bBody = false;
				break;
			}
	}
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
					break ;
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

bool Webserv::needAResponse(size_t i) const
{
	for (std::vector<query>::const_iterator it = m_queries.begin(); it != m_queries.end(); ++it)
		if ((*it).fd == m_fds[i].fd && !(*it).formatedResponse.empty())
			return (true);
	return (false);
}

std::vector<server> Webserv::createServers() const
{
	std::vector<const Node*> _servers;
	std::vector<const Node*> _listens;
	std::vector<const Node*> _roots;
	std::vector<const Node*> _server_names;
	std::vector<const Node*>::const_iterator current_server;
	std::vector<server> servers;
	std::vector<std::string> args;
	uint16_t _port;
	std::string _host;
	server _server;

	_servers = m_parser->getDirectives("server");
	for (std::vector<const Node*>::const_iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		current_server = it;
		_server.ports.clear();
		_server.hosts.clear();
		_server.server_names.clear();
		_server.root = "/html";
		_listens = m_parser->getDirectives("listen", static_cast<const NodeBlock*>(*it));
		for (std::vector<const Node*>::const_iterator it = _listens.begin(); it != _listens.end(); ++it)
		{
			_port = 80;
			_host = "0.0.0.0";
			if (!static_cast<const NodeDirective*>(*it)->getListenHostPort(_port, _host))
			{
				_server.ports.push_back(_port);
				_server.hosts.push_back(_host);
			}
		}
		_roots = m_parser->getDirectives("root", static_cast<const NodeBlock*>(*current_server));
		for (std::vector<const Node*>::const_iterator it = _roots.begin(); it != _roots.end(); ++it)
		{
			args = (*it)->getArgs();
			for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); ++it)
			{
				_server.root = *it;
				std::cout << *it << std::endl;
			}
		}
		_server_names = m_parser->getDirectives("server_name", static_cast<const NodeBlock*>(*current_server));
		for (std::vector<const Node*>::const_iterator it = _server_names.begin(); it != _server_names.end(); ++it)
		{
			args = (*it)->getArgs();
			for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); ++it)
				_server.server_names.push_back(*it);
		}
		servers.push_back(_server);
	}
	return (servers);
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