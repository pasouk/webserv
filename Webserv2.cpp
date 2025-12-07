/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv2.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 10:50:25 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/11/23 10:18:02 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

/*
curl -X POST "http://localhost:8080/cgi-bin/test.bla/coucou?zozo=2" -d "message=Hello+World"
*/
//./tester http://localhost:8080
//curl "http://localhost:4098/cgi-bin/python/cgi1.py/foo/youpla/houp?name=toto&age=12"
//curl "http://localhost:4098/cgi-bin/python/cgi1.py?name=toto&age=12"
int Webserv::responseHook(s_query*& q, void (*onResponse)(std::string&, std::string*, ParserHttpRequest&, s_server&))
{
	std::map<std::string, std::string> headers;
	std::string header, cgiPath, binary;
	std::map<std::string, std::string> env;
	std::ostringstream oss;
	std::stringstream ss;
	s_http_path http_path;
	s_server s;
	const pollfd *fds;
	int ret = 0;

	s = getRightServer(q);
	q->httpParser->setBodyLine(q->bodyChunks);
	http_path = parseHttpPath(s, q->httpParser->getPath());
	if (q->cgi == NULL && isCgi(s, http_path.path_updated, cgiPath, binary))
	{
		env["SCRIPT_NAME"] = http_path.path_updated;
        env["PATH_INFO"] = http_path.path_info;
		env["QUERY_STRING"] = http_path.query_string;
		env["SERVER_PROTOCOL"] = q->httpParser->getVersion();
		ss << q->port;
		env["SERVER_PORT"] = ss.str();
		env["REQUEST_METHOD"] = methods_map[q->httpParser->getMethod()].name;
		env["SERVER_NAME"] = q->hostName;
		env["CONTENT_LENGTH"] = "0";		
		headers = q->httpParser->getHeaders();
		header = headers["Content-Length"]; 
		if (!header.empty())
			env["CONTENT_LENGTH"] = header;
		if (callCGI(cgiPath, env, q, binary))
		{
			oss << "CGI can't be build.:";
			logErrMessage(oss);
			ret = 1;
		}
		else
		{
			fds = q->cgi->getPollfd();
			pollfd (&arr)[2] = *reinterpret_cast<pollfd (*)[2]>(const_cast<pollfd *>(fds));
			addPipeToPoll(arr);
		}
	}
	else
		onResponse(q->formatedResponse, NULL, *(q->httpParser), s);
	m_queries.push_back(*q);
	//printQuery(*q);
	q->httpRequest.clear();
	q->bodySize = 0;
	q->byteSent = 0;
	q->httpParser = NULL;
	q->cgi = NULL;
	q->bodyChunks.clear();
	return (ret);
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

int Webserv::tcpStream(char* buffer, ssize_t n, s_query*& q
	, void (*onResponse)(std::string&, std::string*, ParserHttpRequest&, s_server&), bool& bDelete)
{
	std::map<std::string, std::string> headers;
	std::stringstream ss;
	ssize_t i = 0;
	std::string header;
	std::pair<char*, ssize_t> chunk;


	bDelete = true;
	if (q->bodySize)
	{
		if (i + q->bodySize < n)
		{
			chunk = removeChunk(&buffer[i], q->bodySize);
			q->bodyChunks.push_back(chunk);
			i += q->bodySize;
			if (responseHook(q, onResponse))
				return (1);
		}
		else
		{
			chunk.second = n;
			chunk.first = buffer;
			q->bodyChunks.push_back(chunk);
			bDelete = false;
			q->bodySize -= n;
			if (q->bodySize == 0)
				if(responseHook(q, onResponse))
					return (1);
			i += n;
		}
	}
	while (i < n)
	{
		q->httpRequest += buffer[i];
		if (q->httpRequest.find("\r\n\r\n") != std::string::npos)
		{
			if (i < n)
			{
				q->httpParser = new (std::nothrow)ParserHttpRequest(q->httpRequest);
				if (q->httpParser == NULL)
					return (1);
				headers = q->httpParser->getHeaders();
				header = headers["Content-Length"];
				q->bodySize = 0;
				if (!header.empty())
				{
					ss.clear();
					ss << header;
					ss >> q->bodySize;
				}
				if (q->bodySize > 0)
				{
					if (++i < n)
					{
						if (i + q->bodySize < n)
						{
							chunk = removeChunk(&buffer[i], q->bodySize);
							q->bodyChunks.push_back(chunk);
							i += q->bodySize - 1;
							if (responseHook(q, onResponse))
								return (1);
						}
						else
						{
							chunk = removeChunk(&buffer[i], n - i);
							q->bodyChunks.push_back(chunk);
							q->bodySize -= n - i;
							if (q->bodySize == 0)
								if (responseHook(q, onResponse))
									return (1);
							i = n;
						}
					}
				}
				else
					if (responseHook(q, onResponse))
						return (1);
			}
			else
				if (responseHook(q, onResponse))
					return (1);
		}
		++i;
	}			
	return (0);
}

void Webserv::releaseQueries(int fd)
{
	for (size_t j = 0; j < m_queries.size();)
	{
		if (m_queries[j].fd == fd)
		{
			for (size_t k = 0; k < m_queries[j].bodyChunks.size(); ++k)
				delete [](m_queries[j].bodyChunks[k].first);
			m_queries[j].bodyChunks.clear();
			if (m_queries[j].httpParser)
			{
				delete (m_queries[j].httpParser);
				m_queries[j].httpParser = NULL;
			}
			if (m_queries[j].cgi)
			{
				const pollfd *fds = m_queries[j].cgi->getPollfd();
				pollfd (&arr)[2] = *reinterpret_cast<pollfd (*)[2]>(const_cast<pollfd *>(fds));
				removePipesFromPoll(arr);
				delete (m_queries[j].cgi);
				m_queries[j].cgi = NULL;
			}
			m_queries.erase(m_queries.begin() + j);
		}
		else
			j++;
	}
}

void Webserv::destroyClient(int fd)
{
	close(fd);
	releaseQueries(fd);
	for (size_t j = 0; j < m_clients.size(); ++j)
		if (fd == m_clients[j].fd)
		{
			m_clients.erase(m_clients.begin() + j);
			break ;
		}
	for (size_t i = 0; i < m_fds.size(); ++i)
		if (fd == m_fds[i].fd)
		{
			m_fds.erase(m_fds.begin() + i);
			m_fdType.erase(m_fdType.begin() + i);
			break ;
		}
}

bool Webserv::keepAlive(int fd, double sec)
{
	std::ostringstream oss;
	double delay;
	s_query *client;

	if (getClient(fd, client))
	{
		delay = (std::time(NULL) - client->lifeTime);
		if ( delay >= sec)
		{
			oss << "Client fd:" << client->fd << ", no request for " << delay << " sec ...";
			logOutMessage(oss);
			return (false);
		}
		return (true);
	}
	return (true);
}

bool Webserv::getClient(int fd, s_query*& client)
{
	for (size_t j = 0; j < m_clients.size(); ++j)
		if (fd == m_clients[j].fd)
			return (client = &m_clients[j], true);
	return (false);
}

s_server& Webserv::getRightServer(s_query*& q)
{
	std::map<std::string, std::string> headers;
	std::ostringstream oss;
	s_server* ret = &m_servers[0];
	bool bFind = false;

	for (std::vector<s_server>::iterator s = m_servers.begin(); s != m_servers.end(); ++s)
	{
		for (size_t i = 0; i < (*s).hosts.size(); ++i)
			if ((*s).hosts[i] == q->host && (*s).ports[i] == q->port)
			{
				if (!bFind)
					ret = &*s;
				for (std::vector<std::string>::iterator ser_name = (*s).server_names.begin()
					; ser_name != (*s).server_names.end(); ++ser_name)
				{
					headers = q->httpParser->getHeaders();
					if (matchServerName(headers["Host"], *ser_name))
					{
						q->hostName = *ser_name;
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
	m_fdType.push_back(PIPE);
	m_fds.push_back(poll[1]);
	m_fdType.push_back(PIPE);
}

void Webserv::removePipesFromPoll(pollfd(&poll)[2])
{
	for (size_t i = 0; i < m_fds.size(); ++i)
		if (m_fds[i].fd == poll[1].fd)
		{
			m_fds.erase(m_fds.begin() + i);
			m_fdType.erase(m_fdType.begin() + i);
			break ;
		}
	for (size_t i = 0; i < m_fds.size(); ++i)
		if (m_fds[i].fd == poll[0].fd)
		{
			m_fds.erase(m_fds.begin() + i);
			m_fdType.erase(m_fdType.begin() + i);
			break ;
		}

}

void Webserv::printQuery(s_query& query) const
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

void Webserv::printServer(s_server& server) const
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
	std::cout << std::setw(col1) << "max body:" 
        << "\033[0;36m" << std::setw(col2 + 5) << server.max_body_size << "\033[0m" << std::endl;
}

void Webserv::printServers()
{
	for (std::vector<s_server>::iterator it = m_servers.begin(); it != m_servers.end(); ++it)
	{
		std::cout << "SERVER:\n";
		printServer(*it);
	}
}