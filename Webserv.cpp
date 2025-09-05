/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:29:06 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/04 16:12:36 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Node.hpp"

Webserv::Webserv(ConfigParser* parser) : m_parser(parser)
{
	QueryListener* ql;
	std::vector<const Node*> serveurs;
	std::vector<const Node*> listens;
	std::vector<Node*>::const_iterator it2;
	uint16_t port = 80;
	std::string host = "0.0.0.0";

	if (parser == NULL)
		throw std::runtime_error("No configuration");
	serveurs = parser->getDirectives("server");
	for (std::vector<const Node*>::const_iterator it = serveurs.begin(); it != serveurs.end(); ++it)
	{		
		listens = parser->getDirectives("listen", static_cast<const NodeBlock*>(*it));
		if (listens.size() == 0)
		{
			std::cout << "NO LISTEN\n";
			ql = createListener(port, host);
			if (ql)
				m_listeners.push_back(ql);
		}
		else
		{
			std::cout << "SERVER: " << listens.size() << " LISTENERS\n";
			for (std::vector<const Node*>::const_iterator it = listens.begin(); it != listens.end(); ++it)
			{
				if (!static_cast<const NodeDirective*>(*it)->getListenHostPort(port, host))
				{
					ql = createListener(port, host);
					if (ql)
						m_listeners.push_back(ql);
				}
			}
		}
	}
	std::cout << "NUM OF LISTENERS: " << m_listeners.size() << std::endl;
}

Webserv::~Webserv()
{
	cleanWebserv();
	if (m_queries.size())
		std::cout << "\033[0;33mWarning: \033[0m" << m_queries.size() << " unprocessed queries.\n";
}

const std::vector<query>& Webserv::getQueries() const
{
	return (m_queries);
}

const std::vector<server>& Webserv::getServers() const
{
	return (m_servers);
}

void Webserv::startListening()
{
	pollfd fd;
	query query;
	rlimit limit;

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
			if (m_isClient[i] && (m_fds[i].revents & POLLIN))
				checkQueries(i);
		}
	}
	cleanWebserv();
	std::cout << "Stop listening\n";
}

void Webserv::addClient(size_t i)
{
	sockaddr_in serverAddress;
	socklen_t serverlen;
	query query;
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

void Webserv::checkQueries(size_t i)
{
	sockaddr_in serverAddress;
	socklen_t serverlen;
	char buffer[BUFFER_SIZE];
	int n;
	
	getsockname(m_fds[i].fd, (struct sockaddr*)&serverAddress, &serverlen);
	n = read(m_fds[i].fd, buffer, sizeof(buffer) - 1);
	if (n <= 0)
	{
		std::cout << "Deconnected client fd:" << m_fds[i].fd
			<< ", port:"<< ntohs(serverAddress.sin_port) << std::endl;
		close(m_fds[i].fd);
		m_fds.erase(m_fds.begin() + i);
	}
	else
	{
		buffer[n] = '\0';
		for (std::vector<query>::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
			if (m_fds[i].fd == (*it).fd)
			{
				m_queries.push_back(*it);
				printQuery(*it);
				break;
			}
	}	
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

std::vector<std::string> Webserv::getArgsFromServerDirective(const std::string& directive, uint16_t port, const std::string& host) const
{
	std::vector<const Node*> serveurs;
	std::vector<const Node*> listens;
	std::vector<const Node*> directives;
	std::vector<const Node*>::const_iterator ser;
	std::vector<std::string> args, ret;
	uint16_t _port = 80;
	std::string _host = "0.0.0.0";

	serveurs = m_parser->getDirectives("server");
	for (std::vector<const Node*>::const_iterator it = serveurs.begin(); it != serveurs.end(); ++it)
	{	
		ser = it;
		listens = m_parser->getDirectives("listen", static_cast<const NodeBlock*>(*it));
		for (std::vector<const Node*>::const_iterator it = listens.begin(); it != listens.end(); ++it)
			if (!static_cast<const NodeDirective*>(*it)->getListenHostPort(_port, _host))
				if (host == _host && port == _port)
				{
					directives = m_parser->getDirectives(directive, static_cast<const NodeBlock*>(*ser));
					for (std::vector<const Node*>::const_iterator it = directives.begin(); it != directives.end(); ++it)
					{
						args = (*it)->getArgs();
						for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); ++it)
							ret.push_back(*it);
					}
					return (ret);
				}
	}
	return (ret);
}

void Webserv::printQuery(query& query) const
{
    int col1 = 20;
    int col2 = 20;

    std::cout << std::setw(col1) << "fd:" 
              << "\033[0;36m" << std::setw(col2) << query.fd << "\033[0m" << std::endl;
   	std::cout << std::setw(col1) << "port:" 
              << "\033[0;36m" << std::setw(col2) << query.port << "\033[0m" << std::endl;
    std::cout << std::setw(col1) << "host:" 
              << "\033[0;36m" << std::setw(col2) << query.host << "\033[0m" << std::endl;
	std::cout << std::setw(col1) << "http:" 
              << "\033[0;36m" << std::endl << query.http << "\033[0m" << std::endl;
}