/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv2.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 10:50:25 by fabricebuyl       #+#    #+#             */
/*   Updated: 2026/01/09 14:43:11 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

//CGI TESTS
/*
//GET
curl "http://localhost:8080/cgi-bin/python/add.py?a=5&b=3"

//POST
curl -X POST "http://localhost:8080/cgi_tester/c++?zozo=2" -d "yoyo"
curl -X POST "http://localhost:8080/cgi-bin/test.bla?zozo=2" -d "PATH_INFO is set to /cgi-bin/test.bla"
curl -X POST "http://localhost:8080/cgi-bin/python/hello.py?zozo=2" -d "PATH_INFO is set to /cgi-bin/python/hello.py"
curl -X POST "http://localhost:8080/cgi-bin/python/hello.py/c++?zozo=2" -d "PATH_INFO is set to /c++"
curl -X POST "http://localhost:8080/cgi-bin/test.bla" -d "message=Hello+World"
curl -X POST "http://localhost:8080/cgi-bin/cgi_tester" -d "message=Hello+World"
./tester http://localhost:8080
curl "http://localhost:4098/cgi-bin/python/cgi1.py/foo/youpla/houp?name=toto&age=12"
curl "http://localhost:4098/cgi-bin/python/hello.py/foo/youpla/houp?name=toto&age=12"
curl "http://localhost:4098/cgi-bin/python/cgi1.py?name=toto&age=12"
curl -X POST "http://localhost:8080/cgi-bin/cgi_tester?name=toto&age=12" -d "message=Hello+World"
curl -X POST "http://localhost:8080/cgi-bin/cgi_tester/foo?name=toto&age=12" -d "message=Hello+World"
curl -X POST "http://localhost:8080/cgi-bin/cgi_tester/foo?name=toto&age=12" -d "Hello World !"
*/
int Webserv::responseHook(s_query*& q, void (*onResponse)(std::string&, CGI*, ParserHttpRequest&, s_server&))
{
	std::map<std::string, std::string> headers;
	std::string header, binary;
	std::map<std::string, std::string> env;
	std::ostringstream oss;
	std::stringstream ss;
	s_http_path httpPath;
	s_server s;
	int ret;

	ret = 0;
	q->httpParser->setBodyLine(q->bodyChunks);
	s = getRightServer(q);
	httpPath = getLocationFromServer(s, *q->httpParser);
	if (q->cgi == NULL && (httpPath.location && httpPath.location->is_cgi))
	{
		env["SCRIPT_FILENAME"] = httpPath.path_updated;
        env["PATH_INFO"] = httpPath.path_info;
		if (env["PATH_INFO"].empty())
			env["PATH_INFO"] = httpPath.path_updated;
		env["QUERY_STRING"] = httpPath.query_string;
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
		else
			if (q->bodyChunks.size())
			{
				ssize_t body_size = 0;
				for (size_t i = 0; i < q->bodyChunks.size(); ++i)
					body_size += q->bodyChunks[i].second;
				ss.clear();
				ss.str("");
				ss << body_size;
				env["CONTENT_LENGTH"] = ss.str();
			}
		if (createCGI(httpPath.path_updated, env, q, httpPath.location->cgi_pass))
		{
			oss << "CGI can't be build.:";
			logErrMessage(oss);
			ret = 1;
		}
	}
	onResponse(q->formatedResponse, q->cgi, *(q->httpParser), s);
	m_queries.push_back(*q);
	//printQuery(*q);
	q->httpRequest.clear();
	q->bodySize = 0;
	q->byteSent = 0;
	q->httpParser = NULL;
	q->encoding = NULL;
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
	, void (*onResponse)(std::string&, CGI*, ParserHttpRequest&, s_server&), bool& bDelete)
{
	ssize_t i;

	i = 0;
	bDelete = true;
	if (q->encoding)
	{
		if (q->encoding->loadBody1(buffer, n, q, i, bDelete) == LOAD_CALL_HOOK)
		{
			if (responseHook(q, onResponse))
				return (1);
		}
	}
	while (i < n)
	{
		q->httpRequest += buffer[i];
		if (q->httpRequest.find("\r\n\r\n") != std::string::npos)
		{
			if (q->httpParser == NULL)
			{
				q->httpParser = new (std::nothrow)ParserHttpRequest(q->httpRequest);
				if (q->httpParser == NULL)
					return (1);
			}
			if (q->encoding == NULL)
			{
				q->encoding = bodyManagement(q->bodySize, q->httpParser->getHeaders());
				if (q->encoding == NULL)
					return (1);
			}
			if (q->encoding->loadBody2(buffer, n, q, i) == LOAD_CALL_HOOK)
			{
				if (responseHook(q, onResponse))
					return (1);
			}
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
			if (m_queries[j].encoding)
			{
				delete (m_queries[j].encoding);
				m_queries[j].encoding = NULL;
			}
			if (m_queries[j].cgi)
			{
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
							oss << "conflicting server name \"" << *ser_name << "\", first server will be used";
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