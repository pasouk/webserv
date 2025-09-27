/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv2.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 10:50:25 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/27 12:27:33 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

void Webserv:: queryHook(std::vector<query>::iterator it
	, void (*onQuery)(query&, std::vector<server>&, Webserv*))
{
	onQuery(*it, m_servers, this);
	m_queries.push_back(*it);
	(*it).httpRequest.clear();
	(*it).bodySize = 0;
	(*it).byteSent = 0;
	(*it).bodyChunks.clear();
}

char* Webserv::removeChunk(char* stream, ssize_t size)
{
	char* chunk;

	chunk = new (std::nothrow) char[size + 1];
	if (chunk == NULL)
	{
		cleanWebserv();
		throw std::bad_alloc();
	}
	memcpy(chunk, stream, size);
	chunk[size] = '\0';
	return (chunk);
}

bool Webserv::tcpStream(char* buffer, ssize_t n, std::vector<query>::iterator it
	, void (*onQuery)(query&, std::vector<server>&, Webserv*))
{
	ssize_t i = 0;
	bool bDelete = true;
	char *chunk = NULL;

	if ((*it).bodySize)
	{
		if (i + (*it).bodySize < n)
		{
			chunk = removeChunk(&buffer[i], (*it).bodySize);
			(*it).bodyChunks.push_back(chunk);
			i += (*it).bodySize;
			queryHook(it, onQuery);
		}
		else
		{
			(*it).bodyChunks.push_back(buffer);
			bDelete = false;
			(*it).bodySize -= n;
			if ((*it).bodySize == 0)
				queryHook(it, onQuery);
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
				(*it).bodySize = checkForContentLength(*it);
				if ((*it).bodySize > 0)
				{
					if (++i < n)
					{
						if (i + (*it).bodySize < n)
						{
							chunk = removeChunk(&buffer[i], (*it).bodySize);
							(*it).bodyChunks.push_back(chunk);
							i += (*it).bodySize;
							queryHook(it, onQuery);
						}
						else
						{
							chunk = removeChunk(&buffer[i], n - i);
							(*it).bodyChunks.push_back(chunk);
							(*it).bodySize -= n - i;
							if ((*it).bodySize == 0)
								queryHook(it, onQuery);
							i = n;
						}
					}
				}
				else
					queryHook(it, onQuery);
			}
			else
				queryHook(it, onQuery);
		}
		++i;
	}
	return (bDelete);
}

void Webserv::destroyClientQueries(size_t i)
{
	for (size_t j = 0; j < m_queries.size(); ++j)
	{
		if (m_queries[j].fd == m_fds[i].fd)
		{
			for (size_t k = 0; k < m_queries[j].bodyChunks.size(); ++k)
				delete [](m_queries[j].bodyChunks[k]);
			m_queries.erase(m_queries.begin() + j);
			break ;
		}
	}
}

void Webserv::destroyClient(size_t i)
{
	close(m_fds[i].fd);
	for (size_t j = 0; j < m_clients.size(); ++j)
		if (m_fds[i].fd == m_clients[j].fd)
		{
			m_clients.erase(m_clients.begin() + j);
			break ;
		}
	m_fds.erase(m_fds.begin() + i);
	m_isClient.erase(m_isClient.begin() + i);
}

bool Webserv::keepAlive(size_t i, double sec) const
{
	double delay;
	query client;

	if (getClient(i, client))
	{
		delay = (std::time(NULL) - client.lifeTime);
		if ( delay >= sec)
		{
			std::cout << "No request for " << delay << " sec.\n";
			return (false);
		}
		return (true);
	}
	return (true);
}

bool Webserv::getClient(size_t i, query& client) const
{
	for (size_t j = 0; j < m_clients.size(); ++j)
		if (m_fds[i].fd == m_clients[j].fd)
			return (client = m_clients[j], true);
	return (false);
}

ssize_t Webserv::checkForContentLength(query& query) const
{
	std::stringstream ss;
	std::string len;
	size_t pos, end;
	ssize_t	ssl;

	pos = query.httpRequest.find("Content-Length:");
	if (pos != std::string::npos)
	{
		pos += std::strlen("Content-Length:");
		end = query.httpRequest.find("\r\n", pos);
		if (end != std::string::npos)
		{
			len = query.httpRequest.substr(pos, end - pos);
			ss << len;
			ss >> ssl;
			return (ssl);
		}
	}
	return (0);
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
		std::cout << *it << std::endl;
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