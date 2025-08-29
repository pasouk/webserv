/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QueryListener.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 11:52:16 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/29 10:20:17 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "QueryListener.hpp"
#include "ConfigParser.hpp"

QueryListener::QueryListener()
{
	initListener();
	queriesListen();
}

//release socker
QueryListener::~QueryListener()
{
	stopListening();
	std::cout << "Destructor called\n";
}

//exit queriesListen() and free up resources
void QueryListener::stopListening()
{
	g_listening = false;
}

void QueryListener::initListener()
{
	int opt = 1;
	
	//1. build a socket TCP IPv4 and sets the non bloquing mode and
	// initialize pollfd struct array.
	for (int i = 0; i <= MAX_CLIENTS; i++)
	{
		m_fds[i].fd = -1;
		m_fds[i].events = 0;
		m_fds[i].revents = 0; 
	}
	m_fds[0].fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_fds[0].fd == -1)
		throw std::runtime_error(std::strerror(errno));
	m_fds[0].events = POLLIN | POLLOUT;

	// only F_SETFL, O_NONBLOCK and FD_CLOEXEC can be used (macOS).
	if (fcntl(m_fds[0].fd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(m_fds[0].fd);
		throw std::runtime_error(std::strerror(errno));
	}

	//2. allow port reuse
	if (setsockopt(m_fds[0].fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(m_fds[0].fd);
		throw std::runtime_error(std::strerror(errno));
	}

	//3. port address definition
	memset(&m_serverAddress, 0, sizeof(m_serverAddress));
 	m_serverAddress.sin_family = AF_INET;
	m_serverAddress.sin_port = htons(PORT);
	m_serverAddress.sin_addr.s_addr = INADDR_ANY;

	//4. link address and socket
	if (bind(m_fds[0].fd, (struct sockaddr*)&m_serverAddress, sizeof(m_serverAddress)) < 0)
	{
		close(m_fds[0].fd);
 		throw std::runtime_error(strerror(errno));
	}

    //5. socket accepts incoming connections
    if (listen(m_fds[0].fd, 5) < 0)
	{
		close(m_fds[0].fd);
 		throw std::runtime_error(std::strerror(errno));	
	}
}

void QueryListener::queriesListen()
{
	int				client_fd, ret, n;
	socklen_t serverlen = sizeof(m_serverAddress);
	
	while (g_listening)
	{
		ret = poll(m_fds, MAX_CLIENTS + 1, 0/*no delay*/); 
        if (ret < 0)
		{
			g_listening = false;
			break;
		}
       	if (m_fds[0].revents & POLLIN)
	   	{
        	client_fd = accept(m_fds[0].fd, (struct sockaddr*)&m_serverAddress, &serverlen);
        	if (client_fd < 0) 
				continue;

            // Add to m_fds array
            for (int i = 1; i <= MAX_CLIENTS; i++)
			{
                if (m_fds[i].fd == -1)
				{
                    m_fds[i].fd = client_fd;
                    m_fds[i].events = POLLIN;
					std::cout << "New client connected: n°:" << i << std::endl;
                    break;
                }
				else if (i == MAX_CLIENTS)
					std::cout << "Increase MAX_CLIENTS.\n";
            }
        }

      	// Read/write data clients
        for (int i = 1; i <= MAX_CLIENTS; i++)
		{
            if (m_fds[i].fd != -1 && (m_fds[i].revents & POLLIN))
			{
                char buffer[BUFFER_SIZE];
                n = read(m_fds[i].fd, buffer, sizeof(buffer) - 1);
                if (n <= 0)
				{
                    std::cout << "Deconnected client N°" << i << std::endl;
                    close(m_fds[i].fd);
                    m_fds[i].fd = -1;
                } else
				{
                    buffer[n] = '\0';
                    std::cout << "Client N°" << i <<" send: " << buffer;
                }
            }
            if (m_fds[i].fd != -1 && (m_fds[i].revents & POLLOUT))
			{
			}
        }
	}

	closeFds();
	std::cout << "Stop listening\n";
}

void QueryListener::closeFds()
{		
	for (int i = 0; i <= MAX_CLIENTS; i++)
		if (m_fds[i].fd != -1)
		{
			if (i == 0)
            	std::cout << "Deconnected server\n";
			else
            	std::cout << "Deconnected client N°" << i << std::endl;
			close(m_fds[i].fd);
			m_fds[i].fd = -1;
		}
}
