/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:29:06 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/31 14:50:57 by fbuyl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Node.hpp"

Webserv::Webserv(ConfigParser* parser)
{
	std::vector<Node*> serveurs;
	std::vector<Node*> listens;
	QueryListener *pQL;
	uint16_t port = 80;
	std::string host;

	if (parser == NULL)
		throw std::runtime_error("No configuration");

	serveurs = parser->getDirectives("server");
	for (std::vector<Node*>::const_iterator it = serveurs.begin(); it != serveurs.end(); ++it)
	{		
		listens = parser->getDirectives("listen", static_cast<NodeBlock*>(*it));
		if (listens.size() == 0)
		{
			std::cout << "NO LISTEN\n";
			try
			{
				pQL = new QueryListener(port, host);
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
				cleanWebserv();
				throw std::runtime_error(e.what());
			}
			
			m_listeners.push_back(pQL);
		}
		else
		{
			std::cout << listens.size() << " LISTENERS\n";
			for (std::vector<Node*>::const_iterator it = listens.begin(); it != listens.end(); ++it)
			{
				static_cast<NodeDirective*>(*it)->getHostPort(port, host);
				try
				{
					pQL = new QueryListener(port, host);
				}
				catch(const std::exception& e)
				{
					std::cerr << e.what() << '\n';
					cleanWebserv();
					throw std::runtime_error(e.what());
				}				
				m_listeners.push_back(pQL);
			}
		}
	}
	queriesListen();
	//std::cout << *config;
}

Webserv::~Webserv()
{
	cleanWebserv();
}

void Webserv::queriesListen()
{
	int n;
	size_t j;
	pollfd fd;
	rlimit limit;
	sockaddr_in serverAddress;
	socklen_t serverlen;

	//check system queue size
	if (getrlimit(RLIMIT_NOFILE, &limit) == -1)
	{
		cleanWebserv();
		throw std::runtime_error(std::strerror(errno));	
	}
	for (std::vector<QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
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
			{
				for (std::vector<QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
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
							std::cout << "New client connected: fd:" << fd.fd
								<< ", port:"<< ntohs(serverAddress.sin_port) << std::endl;
						}
						break;
					}
			}
			if (m_isClient[i] && (m_fds[i].revents & POLLIN))
			{
                char buffer[BUFFER_SIZE];
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
                    std::cout << "Client fd:" << m_fds[i].fd <<" send: " << buffer;
                }
			}
		}
	}
	cleanWebserv();
	std::cout << "Stop listening\n";
}

void Webserv::cleanWebserv()
{
	stopListening();
	for (std::vector<QueryListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
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