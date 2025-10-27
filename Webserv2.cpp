/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv2.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 10:50:25 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/10/27 15:27:49 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

int Webserv::responseHook(std::vector<query>::iterator it
	, void (*onResponse)(std::string&, ParserHttpRequest&, server&))
{
	std::map<std::string, std::string> env;
	std::stringstream ss;
	std::ostringstream oss;
	const pollfd *fds;

	(*it).httpParser->setBodyLine((*it).bodyChunks);
	if ((*it).cgi == NULL && isRunnable((*it).httpParser->getPath()))
	{
		ss << (*it).port;
        env["QUERY_STRING"] = "not implemented";
        env["PATH_INFO"] = "not implemented";
		env["SCRIPT_NAME"] = (*it).httpParser->getPath();
        env["REQUEST_METHOD"] = methods_map[(*it).httpParser->getMethod()].name;
		env["SERVER_PORT"] = ss.str();
		env["SERVER_NAME"] = (*it).hostName;
		env["SERVER_SOFTWARE"] = "webserv/1.0";
		env["SERVER_PROTOCOL"] = (*it).httpParser->getVersion();
		if (callCGI((*it).httpParser->getPath(), env, (*it).cgi))
		{
			oss << "CGI can't be build.";
			logErrMessage(oss);
			return (1);
		}
		fds = (*it).cgi->getPoll();
		pollfd (&arr)[2] = *reinterpret_cast<pollfd (*)[2]>(const_cast<pollfd *>(fds));
		addPipeToPoll(arr);
	}
	onResponse((*it).formatedResponse, *((*it).httpParser), getRightServer(*it));
	m_queries.push_back(*it);
	//printQuery(*it);
	(*it).httpRequest.clear();
	(*it).bodySize = 0;
	(*it).byteSent = 0;
	(*it).httpParser = NULL;
	(*it).bodyChunks.clear();
	return (0);
}

std::pair<char*, ssize_t> Webserv::removeChunk(char* stream, ssize_t size)
{
	std::pair<char*, ssize_t> chunk;

	chunk.first = new (std::nothrow) char[size + 1];
	if (chunk.first == NULL)
	{
		cleanWebserv();
		throw std::bad_alloc();
	}
	memcpy(chunk.first, stream, size);
	chunk.first[size] = '\0';
	chunk.second = size;
	return (chunk);
}

int Webserv::tcpStream(char* buffer, ssize_t n, std::vector<query>::iterator it
	, void (*onResponse)(std::string&, ParserHttpRequest&, server&), bool& bDelete)
{
	std::map<std::string, std::string> headers;
	std::stringstream ss;
	ssize_t i = 0;
	std::string header;
	std::pair<char*, ssize_t> chunk;

	bDelete = true;
	if ((*it).bodySize)
	{
		if (i + (*it).bodySize < n)
		{
			chunk = removeChunk(&buffer[i], (*it).bodySize);
			(*it).bodyChunks.push_back(chunk);
			i += (*it).bodySize;
			if (responseHook(it, onResponse))
				return (1);
		}
		else
		{
			chunk.second = n;
			chunk.first = buffer;
			(*it).bodyChunks.push_back(chunk);
			bDelete = false;
			(*it).bodySize -= n;
			if ((*it).bodySize == 0)
				if(responseHook(it, onResponse))
					return (1);
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
				(*it).httpParser = new (std::nothrow)ParserHttpRequest((*it).httpRequest);
				if ((*it).httpParser == NULL)
					return (1);
				headers = (*it).httpParser->getHeaders();
				header = headers["Content-Length"];
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
							if (responseHook(it, onResponse))
								return (1);
						}
						else
						{
							chunk = removeChunk(&buffer[i], n - i);
							(*it).bodyChunks.push_back(chunk);
							(*it).bodySize -= n - i;
							if ((*it).bodySize == 0)
								if (responseHook(it, onResponse))
									return (1);
							i = n;
						}
					}
				}
				else
					if (responseHook(it, onResponse))
						return (1);
			}
			else
				if (responseHook(it, onResponse))
					return (1);
		}
		++i;
	}			
	return (0);
}

void Webserv::destroyClientQueries(size_t i)
{
	for (size_t j = 0; j < m_queries.size(); ++j)
	{
		if (m_queries[j].fd == m_fds[i].fd)// && m_queries[j].cgi == NULL)
		{
			for (size_t k = 0; k < m_queries[j].bodyChunks.size(); ++k)
				delete [](m_queries[j].bodyChunks[k].first);
			if (m_queries[j].httpParser)
			{
				delete (m_queries[j].httpParser);
				m_queries[j].httpParser = NULL;
			}
			/*if (m_queries[j].cgi)
			{
				delete (m_queries[j].cgi);
				m_queries[j].cgi = NULL;
			}*/
			m_queries.erase(m_queries.begin() + j);
		}
	}
}

void Webserv::destroyClient(size_t i)
{
	close(m_fds[i].fd);
	destroyClientQueries(i);
	for (size_t j = 0; j < m_clients.size(); ++j)
		if (m_fds[i].fd == m_clients[j].fd)
		{
			for (size_t k = 0; k < m_clients[j].bodyChunks.size(); ++k)
				delete [](m_clients[j].bodyChunks[k].first);
			if (m_clients[j].httpParser)
			{
				delete (m_clients[j].httpParser);
				m_clients[j].httpParser = NULL;
			}
			if (m_clients[j].cgi)
			{
				delete (m_clients[j].cgi);
				m_clients[j].cgi = NULL;
			}
			m_clients.erase(m_clients.begin() + j);
			break ;
		}
	m_fds.erase(m_fds.begin() + i);
	m_isClient.erase(m_isClient.begin() + i);
}

bool Webserv::keepAlive(size_t i, double sec) const
{
	std::ostringstream oss;
	double delay;
	query client;

	if (getClient(i, client))
	{
		delay = (std::time(NULL) - client.lifeTime);
		if ( delay >= sec)
		{
			oss << "Client fd:" << client.fd << ", no request for " << delay << " sec ...";
			logOutMessage(oss);
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

server& Webserv::getRightServer(query& q)
{
	std::map<std::string, std::string> headers;
	std::ostringstream oss;
	server* ret = &m_servers[0];
	bool bFind = false;

	for (std::vector<server>::iterator s = m_servers.begin(); s != m_servers.end(); ++s)
	{
		for (size_t i = 0; i < (*s).hosts.size(); ++i)
			if ((*s).hosts[i] == q.host && (*s).ports[i] == q.port)
			{
				if (!bFind)
					ret = &*s;
				for (std::vector<std::string>::iterator ser_name = (*s).server_names.begin()
					; ser_name != (*s).server_names.end(); ++ser_name)
				{
					headers = q.httpParser->getHeaders();
					if (matchServerName(headers["Host"], *ser_name))
					{
						q.hostName = *ser_name;
						if (bFind)
						{
							oss << "conflicting server name \"" << *ser_name << "\", first server will be ignored";
							logOutMessage(oss);
							return (*ret);
						}
						bFind = true;
						ret = &*s;
					}
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
	if (_host == _ser)
		return (true);
	return (false);
}

void Webserv::addPipeToPoll(pollfd(&poll)[2])
{
	m_fds.push_back(poll[0]);
	m_isClient.push_back(false);
	m_fds.push_back(poll[1]);
	m_isClient.push_back(false);
}

void Webserv::removePipeFromPoll(pollfd(&poll)[2])
{
	for (size_t i = 0; i < m_fds.size(); ++i)
		if (m_fds[i].fd == poll[1].fd)
		{
			std::cout << "FIND IT AND REMOVE\n";
			m_fds.erase(m_fds.begin() + i);
			m_isClient.erase(m_isClient.begin() + i);
			break ;
		}
	for (size_t i = 0; i < m_fds.size(); ++i)
		if (m_fds[i].fd == poll[0].fd)
		{
			std::cout << "FIND IT AND REMOVE\n";
			m_fds.erase(m_fds.begin() + i);
			m_isClient.erase(m_isClient.begin() + i);
			break ;
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
	for (std::deque<std::pair<char*, ssize_t> >::iterator it = query.bodyChunks.begin(); it != query.bodyChunks.end()
		; ++it)
		std::cout << (*it).first << ", size of: " << (*it).second << std::endl;
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