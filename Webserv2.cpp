/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv2.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 10:50:25 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/10/05 15:09:59 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

void Webserv:: queryHook(std::vector<query>::iterator it
	, void (*onQuery)(query&, const server&, Webserv*))
{
	onQuery(*it, getRightServer(*it), this);
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
	, void (*onQuery)(query&, const server&, Webserv*))
{
	std::stringstream ss;
	ssize_t i = 0;
	std::string header;
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
				header = getHttpHeaderValue(*it, "Content-Length");
				(*it).bodySize = 0;
				if (!header.empty())
				{
					ss.clear();
					ss << header;
					ss >> (*it).bodySize;
				}
				if ((*it).bodySize > 0)
				{
					if (++i < n)
					{
						if (i + (*it).bodySize < n)
						{
							chunk = removeChunk(&buffer[i], (*it).bodySize);
							(*it).bodyChunks.push_back(chunk);
							i += (*it).bodySize - 1;
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
			//break ;
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
			std::cout << "Client fd:" << client.fd << ", no request for " << delay << " sec.\n";
			return (false);
		}
		return (true);
	}
	return (true);
}

bool Webserv::clientAsksClose(size_t i)
{
	query client;
	
	if (getClient(i, client))
	{
		std::string header = getHttpHeaderValue(client, "Connection");
		if (header == "close")
		{
			std::cout << "Client fd:" << client.fd << ", asked to close connexion.\n";
			return (true);
		}
	}
	return (false);
}

bool Webserv::getClient(size_t i, query& client) const
{
	for (size_t j = 0; j < m_clients.size(); ++j)
		if (m_fds[i].fd == m_clients[j].fd)
			return (client = m_clients[j], true);
	return (false);
}

const std::string Webserv::getHttpHeaderValue(query& query, std::string header) const
{
	std::string ret;
	size_t pos, end;

	header = header + ":";
	pos = query.httpRequest.find(header);
	if (pos != std::string::npos)
	{
		pos += std::strlen(header.c_str()) + 1;
		end = query.httpRequest.find("\r\n", pos);
		if (end != std::string::npos)
			return (ret = query.httpRequest.substr(pos, end - pos), ret);
	}
	return (ret);
}

const server& Webserv::getRightServer(query& q) const
{
	const server* ret = &m_servers[0];

	for (std::vector<server>::const_iterator s = m_servers.begin(); s != m_servers.end(); ++s)
	{
		for (size_t i = 0; i < (*s).hosts.size(); ++i)
			if ((*s).hosts[i] == q.host && (*s).ports[i] == q.port)
			{
				ret = &*s;
				for (std::vector<std::string>::const_iterator ser_name = (*s).server_names.begin()
					; ser_name != (*s).server_names.end(); ++ser_name)
				{
					if (matchServerName(getHttpHeaderValue(q, "Host"), *ser_name))
						return (*ret);
				}
			}
	}
	return (*ret);
}

bool Webserv::matchServerName(const std::string& host, const std::string& ser) const
{
	(void)ser;
	size_t pos;
	std::string _host;
	std::string _ser;

	_host = host;
	_ser = ser;
	pos = host.find(":");
	if (pos != std::string::npos)
		_host = host.substr(0, pos);
	for (std::string::iterator c = _host.begin(); c != _host.end(); ++c)
		*c = std::tolower(*c);
	for (std::string::iterator c = _ser.begin(); c != _ser.end(); ++c)
		*c = std::tolower(*c);
	std::cout << "SERVER: " <<ser << " -> " << "HOST: " << _host << std::endl;	
	if (_host == _ser)
		return (std::cout << "YEAH !\n", true);
	return (false);
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
	for (std::vector<std::string>::const_iterator it = server.server_names.begin(); it != server.server_names.end(); ++it)
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