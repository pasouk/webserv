/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:29:06 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/19 15:34:44 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Node.hpp"

Webserv::Webserv(ConfigParser* parser) : m_parser(parser)
{
	QueryListener* ql;
	std::vector<std::string> args;
	std::vector<const Node*> servers;
	std::vector<const Node*> clientBufferSize;
	std::vector<const Node*> listens;
	std::vector<Node*>::const_iterator it2;
	uint16_t port;
	std::string host;

	m_client_buffers_size[0] = HEADER_BUFFER_SIZE;
	m_client_buffers_size[1] = BODY_BUFFER_SIZE;
	if (parser == NULL)
		throw std::runtime_error("No configuration");
	m_servers = findServers();
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
	clientBufferSize = parser->getDirectives("client_body_buffer_size");
	for (std::vector<const Node*>::const_iterator it = clientBufferSize.begin(); it != clientBufferSize.end(); ++it)
		static_cast<const NodeDirective*>(*it)->getClientBufferSize(m_client_buffers_size[1]);
	clientBufferSize = parser->getDirectives("client_header_buffer_size");
	for (std::vector<const Node*>::const_iterator it = clientBufferSize.begin(); it != clientBufferSize.end(); ++it)
		static_cast<const NodeDirective*>(*it)->getClientBufferSize(m_client_buffers_size[0]);
	std::cout << "buffer Header_size: " << m_client_buffers_size[0] << std::endl;
	std::cout << "buffer Body_size: " << m_client_buffers_size[1] << std::endl;
}

Webserv::~Webserv()
{
	cleanWebserv();
	if (m_queries.size())
		std::cout << "\033[0;33mWarning: \033[0m" << m_queries.size() << " unprocessed queries.\n";
}

void Webserv::startListening(void (*onContentLength)(query&, Webserv*)
	, void (*onQueries)(std::vector<query>&, std::vector<server>&, Webserv*))
{
	pollfd fd;
	static query query;
	static rlimit limit;

	//check system queue size
	if (getrlimit(RLIMIT_NOFILE, &limit) == -1)
	{
		cleanWebserv();
		throw std::runtime_error(std::strerror(errno));	
	}
	for (std::vector<const QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{	//add listing descriptor to queue
		fd.fd = (*it)->getListenFD();
		fd.events = POLLIN;
		fd.revents = 0;
		m_fds.push_back(fd);
		m_isClient.push_back(false);
	}
	std::cout << "Listening...\n";
	while (g_listening)
	{
		if (poll(reinterpret_cast<pollfd*>(m_fds.data()), m_fds.size(), 0) < 0)
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
					readQuery(i, onContentLength, onQueries);
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
	pollfd fd;
	size_t j;
	char ip[INET_ADDRSTRLEN];

	for (std::vector<const QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{					
		serverAddress = (*it)->getServerAddress();
		serverlen = sizeof(serverAddress);
		fd.fd = accept(m_fds[i].fd, (struct sockaddr*)&serverAddress, &serverlen);
		if (fd.fd < 0) 
			continue;
		fd.events = POLLIN;
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

void Webserv::readQuery(size_t i, void (*onContentLength)(query&, Webserv*)
	, void (*onQueries)(std::vector<query>&, std::vector<server>&, Webserv*))
{
	static sockaddr_in serverAddress;
	static socklen_t serverlen;
	static char *buffer;
	static bool bBody;
	ssize_t n;
	
	buffer = new (std::nothrow) char[m_client_buffers_size[bBody]];
	if (buffer == NULL)
	{
		cleanWebserv();
		throw std::bad_alloc();
	}
	getsockname(m_fds[i].fd, (struct sockaddr*)&serverAddress, &serverlen);
	n = read(m_fds[i].fd, buffer, m_client_buffers_size[bBody] - 1);
	if (n <= 0)
	{
		std::cout << "Deconnected client fd:" << m_fds[i].fd
			<< ", port:"<< ntohs(serverAddress.sin_port) << std::endl;
		close(m_fds[i].fd);
		for (size_t j = 0; j < m_clients.size(); ++j)
			if (m_fds[i].fd == m_clients[j].fd)
			{
				m_clients.erase(m_clients.begin() + j);
				break ;
			}
		for (size_t j = 0; j < m_queries.size(); ++j)
			if (m_fds[i].fd == m_queries[j].fd)
			{
				m_queries.erase(m_queries.begin() + j);
				break ;
			}	
		m_fds.erase(m_fds.begin() + i);
		m_isClient.erase(m_isClient.begin() + i);
	}
	else
	{
		buffer[n] = '\0';
		for (std::vector<query>::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
		{
			if (m_fds[i].fd == (*it).fd)
			{
				tcpStream(buffer, n, it, onContentLength);
				bBody = false;
				if ((*it).bodySize)
					bBody = true;
				break;
			}
		}
		onQueries(m_queries, m_servers, this);
	}
	//delete [](buffer);
}

void Webserv::tcpStream(char* buffer, ssize_t n, std::vector<query>::iterator it
	, void (*onContentLength)(query&, Webserv*))
{
	ssize_t i = 0;

	if ((*it).bodySize)
	{
		if (i + (*it).bodySize < (size_t)n)
		{
			char *body = new (std::nothrow) char[(*it).bodySize];
			memcpy(body, &buffer[i], (*it).bodySize);
			(*it).bodyChunks.push_back(body);
			i += (*it).bodySize;
			m_queries.push_back(*it);
			(*it).httpRequest.clear();
			(*it).bodySize = 0;
			(*it).bodyChunks.clear();
		}
		else
		{
			(*it).bodyChunks.push_back(buffer);
			(*it).bodySize -= n;
			if ((*it).bodySize == 0)
			{
				m_queries.push_back(*it);
				(*it).httpRequest.clear();
				(*it).bodyChunks.clear();
			}
			i += n;
		}
	}
	while (i < n)
	{
		(*it).httpRequest += buffer[i];
		if ((*it).httpRequest.find("\r\n\r\n") != std::string::npos)
		{
			if (i < n)
			{
				onContentLength(*it, this);
				if ((*it).bodySize > 0)
				{
					if (++i < n)
					{
						if (i + (*it).bodySize < (size_t)n)
						{
							char *body = new (std::nothrow) char[(*it).bodySize];
							memcpy(body, &buffer[i], (*it).bodySize);
							(*it).bodyChunks.push_back(body);
							i += (*it).bodySize;
							m_queries.push_back(*it);
							(*it).httpRequest.clear();
							(*it).bodySize = 0;
							(*it).bodyChunks.clear();
						}
						else
						{
							char *body = new (std::nothrow) char[n - i];
							memcpy(body, &buffer[i], n - i);
							(*it).bodyChunks.push_back(body);
							(*it).bodySize -= n - i;
							if ((*it).bodySize == 0)
							{
								m_queries.push_back(*it);
								(*it).httpRequest.clear();
								(*it).bodyChunks.clear();
							}
							i = n;
						}
					}
				}
				else
				{
					m_queries.push_back(*it);
					(*it).httpRequest.clear();
					(*it).bodyChunks.clear();
				}
			}
			else
			{
				m_queries.push_back(*it);
				(*it).httpRequest.clear();
				(*it).bodyChunks.clear();
				(*it).bodySize = 0;
			}
		}
		++i;
	}
}


void Webserv::sendQuery(size_t i)
{
	(void)i;

	for (std::vector<query>::iterator it = m_queries.begin(); it != m_queries.end(); ++it)
		if (m_fds[i].fd == (*it).fd)
		{
			//while ()
		}
}

std::vector<server> Webserv::findServers() const
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
		_server.root = "";
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
				_server.root = *it;		
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
			close (m_fds[i].fd);
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

void Webserv::printQuery(query& query) const
{
    int col1 = 11;
    int col2 = 20;

    std::cout << std::setw(col1) << "fd:" 
              << "\033[0;36m" << std::setw(col2) << query.fd << "\033[0m" << std::endl;
   	std::cout << std::setw(col1) << "port:" 
              << "\033[0;36m" << std::setw(col2) << query.port << "\033[0m" << std::endl;
    std::cout << std::setw(col1) << "host:" 
              << "\033[0;36m" << std::setw(col2) << query.host << "\033[0m" << std::endl;
	std::cout << std::setw(col1) << "http:" 
              << "\033[0;36m" << std::endl << query.httpRequest << "\033[0m" << std::endl;
	std::cout << std::setw(col1) << "bodyChunks:" 
              << "\033[0;36m" << std::endl;
	for (std::deque<char*>::iterator it = query.bodyChunks.begin(); it != query.bodyChunks.end()
		; ++it)
		std::cout << "\033[0m" << *it << std::endl;
	std::cout << "\033[0m" << ", num chunck: " << query.bodyChunks.size() << std::endl;
}

void Webserv::printServer(server& server) const
{
    int col1 = 15;
    int col2 = 15;

	std::cout << std::setw(col1) << "server_name:" << "\033[0;36m";
	for (std::vector<std::string>::iterator it = server.server_names.begin(); it != server.server_names.end(); ++it)
		std::cout << std::setw(col2) << *it << " ";
	std::cout << "\033[0m" << std::endl;
	std::cout << std::setw(col1) << "port:" << "\033[0;36m";
	for (std::vector<uint16_t>::iterator it = server.ports.begin(); it != server.ports.end(); ++it)
		std::cout << std::setw(col2) << *it << " ";
	std::cout << "\033[0m" << std::endl;
	std::cout << std::setw(col1) << "host:" << "\033[0;36m";
	for (std::vector<std::string>::iterator it = server.hosts.begin(); it != server.hosts.end(); ++it)
		std::cout << std::setw(col2) << *it << " ";
	std::cout << "\033[0m" << std::endl;
	std::cout << std::setw(col1) << "root:" 
        << "\033[0;36m" << std::setw(col2 + 5) << server.root << "\033[0m" << std::endl;
}

void Webserv::printServers()
{
	for (std::vector<server>::iterator it = m_servers.begin(); it != m_servers.end(); ++it)
	{
		std::cout << "SERVER:\n";
		printServer(*it);
	}
}