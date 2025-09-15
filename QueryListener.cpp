/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QueryListener.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 11:52:16 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/06 13:24:15 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "QueryListener.hpp"
#include "ConfigParser.hpp"

QueryListener::QueryListener(uint16_t port, const std::string& host)
{
	initListener(port, host);
}

//release socker
QueryListener::~QueryListener()
{
	close(m_listenFD);
	std::cout << "Deconnected server\n";
	std::cout << "Destructor called\n";
}

int QueryListener::getListenFD() const
{
	return (m_listenFD);
}

const struct sockaddr_in& QueryListener::getServerAddress() const
{
	return (m_serverAddress);
}

void QueryListener::initListener(uint16_t port, const std::string& host)
{
	int opt = 1;

	//1. build a socket TCP IPv4 and sets the non bloquing mode and
	// initialize pollfd struct array.
	m_listenFD = socket(AF_INET, SOCK_STREAM, 0);
	if (m_listenFD == -1)
		throw std::runtime_error(std::strerror(errno));

	// only F_SETFL, O_NONBLOCK and FD_CLOEXEC can be used (macOS).
	if (fcntl(m_listenFD, F_SETFL, O_NONBLOCK) == -1)
	{
		close(m_listenFD);
		throw std::runtime_error(std::strerror(errno));
	}

	//2. allow port reuse
	if (setsockopt(m_listenFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(m_listenFD);
		throw std::runtime_error(std::strerror(errno));
	}

	//3. port address definition
	memset(&m_serverAddress, 0, sizeof(m_serverAddress));
 	m_serverAddress.sin_family = AF_INET;
	m_serverAddress.sin_port = htons(port);
	inet_pton(AF_INET, host.c_str(), &m_serverAddress.sin_addr);

	//4. link address and socket
	if (bind(m_listenFD, (struct sockaddr*)&m_serverAddress, sizeof(m_serverAddress)) < 0)
	{
		close(m_listenFD);
 		throw std::runtime_error(strerror(errno));
	}

    //5. socket accepts incoming connections
    if (listen(m_listenFD, 5) < 0)
	{
		close(m_listenFD);
 		throw std::runtime_error(std::strerror(errno));	
	}
}