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
POST /directory/youpi.bla HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Content-Type: test/file
Accept-Encoding: gzip
*/



//CGI TESTS
/*
//GET
curl "http://localhost:8080/cgi-bin/python/add.py?a=5&b=3"

//POST
curl -X POST "http://localhost:8080/directory/youpi.bla/c++?zozo=2" -d "yoyo"
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
	std::string header, /*cgiPath,*/ binary;
	std::map<std::string, std::string> env;
	std::ostringstream oss;
	std::stringstream ss;
	s_http_path httpPath;
	//s_location* l;
	s_server s;
	int ret;

	ret = 0;
	q->httpParser->setBodyLine(q->bodyChunks);
	s = getRightServer(q);
	//l = getLocationFromServer(s, *q->httpParser);
	httpPath = getLocationFromServer(s, *q->httpParser);
	//if (l != NULL)
	//	std::cout << "LOCATION IS: " << l->concatOrReplace << std::endl;
	//httpPath = parseHttpPath(l, s, q->httpParser->getPath());
	if (q->cgi == NULL && (httpPath.location && httpPath.location->is_cgi))//isCgi(l, s, httpPath.path_updated, cgiPath, binary))
	{

		std::cout << "LET BUILD A CGI\n";


		env["SCRIPT_FILENAME"] = httpPath.path_updated;//cgiPath;
        // PATH_INFO should be the original request path (before alias/root substitution)
        env["PATH_INFO"] = q->httpParser->getPath();
		env["QUERY_STRING"] = httpPath.query_string;
		// REQUEST_URI is the original request path + query string
		env["REQUEST_URI"] = q->httpParser->getPath();
		if (!httpPath.query_string.empty())
			env["REQUEST_URI"] += "?" + httpPath.query_string;

		std::cout << "[CGI ENV] SCRIPT_FILENAME=" << env["SCRIPT_FILENAME"] << std::endl;
		std::cout << "[CGI ENV] PATH_INFO=" << env["PATH_INFO"] << std::endl;
		std::cout << "[CGI ENV] REQUEST_URI=" << env["REQUEST_URI"] << std::endl;
		std::cout << "[CGI ENV] QUERY_STRING=" << env["QUERY_STRING"] << std::endl;
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
		// For chunked encoding, use totalBodyWritten instead of Content-Length header
		else if (q->chunkedEncoding && q->totalBodyWritten > 0)
		{
			ss.str("");
			ss.clear();
			ss << q->totalBodyWritten;
			env["CONTENT_LENGTH"] = ss.str();
			std::cout << "[CGI ENV] Setting CONTENT_LENGTH=" << q->totalBodyWritten << " from chunked body" << std::endl;
		}

		// Pass all HTTP headers to CGI with HTTP_ prefix (RFC 3875)
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		{
			std::string headerName = it->first;
			std::string headerValue = it->second;

			// Skip Content-Length and Content-Type as they have special CGI variables
			if (headerName == "Content-Length" || headerName == "Content-Type")
				continue;

			// Convert header name to CGI format: HTTP_ prefix + uppercase + replace - with _
			std::string cgiVarName = "HTTP_";
			for (size_t i = 0; i < headerName.length(); ++i)
			{
				char c = headerName[i];
				if (c == '-')
					cgiVarName += '_';
				else if (c >= 'a' && c <= 'z')
					cgiVarName += (c - 'a' + 'A');  // Convert to uppercase
				else if (c >= 'A' && c <= 'Z')
					cgiVarName += c;
				else
					cgiVarName += c;
			}

			env[cgiVarName] = headerValue;
			std::cout << "[CGI ENV] " << cgiVarName << "=" << headerValue << std::endl;
		}

		// Handle Content-Type separately
		header = headers["Content-Type"];
		if (!header.empty())
			env["CONTENT_TYPE"] = header;
		if (createCGI(httpPath.path_updated/*cgiPath*/, env, q, /*binary*/httpPath.location->cgi_pass))
		{
			oss << "CGI can't be build.:";
			logErrMessage(oss);
			ret = 1;
		}
	}
	onResponse(q->formatedResponse, q->cgi, *(q->httpParser), s);
	std::cout << "[RESPONSE HOOK] Pushing query to m_queries, response size=" << q->formatedResponse.size() << std::endl;
	m_queries.push_back(*q);
	std::cout << "[RESPONSE HOOK] m_queries size now=" << m_queries.size() << std::endl;
	//printQuery(*q);
	q->httpRequest.clear();
	q->bodySize = 0;
	q->byteSent = 0;
	q->httpParser = NULL;
	q->cgi = NULL;
	q->bodyChunks.clear();
	q->chunkedEncoding = false;
	q->currentChunkSize = 0;
	q->readingChunkSize = true;
	q->chunkBuffer.clear();
	// DON'T close bodyFileFd here - CGI still needs it!
	// It will be closed when the client disconnects
	// if (q->bodyFileFd != -1)
	// {
	// 	close(q->bodyFileFd);
	// 	q->bodyFileFd = -1;
	// }
	q->totalBodyWritten = 0;
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

int Webserv::processChunkedData(char* buffer, ssize_t n, s_query*& q
	, void (*onResponse)(std::string&, CGI*, ParserHttpRequest&, s_server&), ssize_t& i, bool&)
{
	// Create temp file on first call
	if (q->bodyFileFd == -1)
	{
		// Use open() with O_TMPFILE or mkstemp() to create a real temporary file
		// that won't be closed when FILE* goes out of scope
		char tmpTemplate[] = "/tmp/webserv_body_XXXXXX";
		q->bodyFileFd = mkstemp(tmpTemplate);
		if (q->bodyFileFd == -1)
		{
			std::cerr << "[ERROR] Failed to create temp file for chunked body: " << strerror(errno) << std::endl;
			return (1);
		}
		// Unlink immediately so file is deleted when fd is closed
		unlink(tmpTemplate);
		std::cout << "[CHUNKED] Created temp file fd=" << q->bodyFileFd << std::endl;
	}

	while (i < n)
	{
		if (q->readingChunkSize)
		{
			// Read chunk size line (hex number followed by \r\n)
			while (i < n)
			{
				char c = buffer[i++];
				if (c == '\r')
					continue;
				if (c == '\n')
				{
					// Ignore empty lines (trailing \r\n after chunk data)
					if (q->chunkBuffer.empty())
						continue;

					// Parse chunk size from hex
					std::stringstream ss;
					ss << std::hex << q->chunkBuffer;
					ss >> q->currentChunkSize;
					std::cout << "[CHUNKED] Parsed chunk size: '" << q->chunkBuffer << "' = " << q->currentChunkSize << " bytes" << std::endl;
					q->chunkBuffer.clear();
					q->readingChunkSize = false;

					// If chunk size is 0, we're done
					if (q->currentChunkSize == 0)
					{
						std::cout << "[CHUNKED] Final chunk received, total=" << q->totalBodyWritten << " bytes, calling responseHook" << std::endl;
						// Flush and rewind temp file to beginning for CGI to read
						fsync(q->bodyFileFd);
						lseek(q->bodyFileFd, 0, SEEK_SET);
						// DEBUG: Check file size
						off_t fileSize = lseek(q->bodyFileFd, 0, SEEK_END);
						lseek(q->bodyFileFd, 0, SEEK_SET);
						std::cout << "[CHUNKED] Temp file flushed and rewound, fd=" << q->bodyFileFd << ", file size=" << fileSize << " bytes" << std::endl;
						if (responseHook(q, onResponse))
							return (1);
						return (0);
					}
					break;
				}
				q->chunkBuffer += c;
			}
		}
		else
		{
			// Read chunk data and write directly to temp file
			ssize_t remaining = q->currentChunkSize;
			ssize_t available = n - i;
			ssize_t toRead = (remaining < available) ? remaining : available;

			if (toRead > 0)
			{
				// Check if writing this chunk would exceed the max body size
				if (q->maxBodySize > 0 && q->totalBodyWritten + toRead > q->maxBodySize)
				{
					std::cout << "[CHUNKED] Body size limit exceeded: " << (q->totalBodyWritten + toRead)
					          << " > " << q->maxBodySize << std::endl;

					// Generate 413 error response (simple, no body)
					q->formatedResponse = "HTTP/1.1 413 Request Entity Too Large\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";
					m_queries.push_back(*q);
					std::cout << "[CHUNKED] Sent 413 Request Entity Too Large" << std::endl;

					// Clean up temp file
					if (q->bodyFileFd != -1)
					{
						close(q->bodyFileFd);
						q->bodyFileFd = -1;
					}

					// Reset chunked state so we don't continue processing
					q->chunkedEncoding = false;
					q->readingChunkSize = true;
					q->currentChunkSize = 0;
					q->totalBodyWritten = 0;

					return (0); // Return success so the error response is sent
				}

				ssize_t written = write(q->bodyFileFd, &buffer[i], toRead);
				if (written < 0)
				{
					std::cerr << "[ERROR] Failed to write to temp file: " << strerror(errno) << std::endl;
					return (1);
				}
				if (q->totalBodyWritten == 0)
					std::cout << "[CHUNKED] First write: fd=" << q->bodyFileFd << ", written=" << written << " bytes" << std::endl;
				q->totalBodyWritten += written;
				i += toRead;
				q->currentChunkSize -= toRead;
			}

			// If we finished reading this chunk, prepare for next chunk size
			// (the trailing \r\n will be skipped as an empty line)
			if (q->currentChunkSize == 0)
			{
				q->readingChunkSize = true;
			}
			else
			{
				// Need more data for this chunk
				break;
			}
		}
	}
	return (0);
}

int Webserv::tcpStream(char* buffer, ssize_t n, s_query*& q
	, void (*onResponse)(std::string&, CGI*, ParserHttpRequest&, s_server&), bool& bDelete)
{
	std::map<std::string, std::string> headers;
	std::stringstream ss;
	ssize_t i = 0;
	std::string header;
	std::pair<char*, ssize_t> chunk;

	bDelete = true;

	// If there's already a response ready (e.g., 413 error), ignore further input
	if (!q->formatedResponse.empty())
	{
		std::cout << "[DEBUG] Ignoring input - response already queued" << std::endl;
		return (0);
	}

	// Handle chunked encoding continuation
	if (q->chunkedEncoding && q->httpParser != NULL)
	{
		return processChunkedData(buffer, n, q, onResponse, i, bDelete);
	}

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

				// Check for Transfer-Encoding: chunked
				std::string transferEncoding = headers["Transfer-Encoding"];
				std::cout << "[DEBUG] Transfer-Encoding: '" << transferEncoding << "'" << std::endl;
				if (transferEncoding == "chunked")
				{
					q->chunkedEncoding = true;
					q->readingChunkSize = true;
					q->currentChunkSize = 0;
					std::cout << "Transfer-Encoding: chunked detected" << std::endl;
				}

				header = headers["Content-Length"];
				q->bodySize = 0;
				if (!header.empty())
				{
					ss.clear();
					ss << header;
					ss >> q->bodySize;
					std::cout << "Content-Length: " << q->bodySize << std::endl;
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
				else if (q->chunkedEncoding)
				{
					// Start reading chunked body
					std::cout << "[DEBUG] Starting chunked body read, chunkedEncoding=" << q->chunkedEncoding << std::endl;

					// Find the max body size for this location
					s_server s = getRightServer(q);
					s_http_path httpPath = getLocationFromServer(s, *q->httpParser);

					// Debug: show what we have
					if (httpPath.location)
					{
						std::cout << "[DEBUG] Location max_body_size value: '" << httpPath.location->max_body_size << "'" << std::endl;
						std::cout << "[DEBUG] Location is_cgi: " << httpPath.location->is_cgi << std::endl;
					}
					else
					{
						std::cout << "[DEBUG] No location matched" << std::endl;
					}
					std::cout << "[DEBUG] Server max_body_size value: '" << s.max_body_size << "'" << std::endl;

					// For CGI requests, don't enforce client_max_body_size - let the CGI handle it
					if (httpPath.location && httpPath.location->is_cgi)
					{
						q->maxBodySize = 0; // 0 means no limit
						std::cout << "[DEBUG] CGI request - no body size limit" << std::endl;
					}
					// Get max_body_size from location or server for non-CGI requests
					else if (httpPath.location && !httpPath.location->max_body_size.empty() && httpPath.location->max_body_size != "not define")
					{
						std::cout << "[DEBUG] Using location max_body_size: '" << httpPath.location->max_body_size << "'" << std::endl;
						std::stringstream ss_size;
						ss_size << httpPath.location->max_body_size;
						ss_size >> q->maxBodySize;
					}
					else if (!s.max_body_size.empty() && s.max_body_size != "not define")
					{
						std::cout << "[DEBUG] Using server max_body_size: '" << s.max_body_size << "'" << std::endl;
						std::stringstream ss_size;
						ss_size << s.max_body_size;
						ss_size >> q->maxBodySize;
					}
					else
					{
						std::cout << "[DEBUG] Using default max_body_size: 8192" << std::endl;
						q->maxBodySize = 8192; // Default
					}
					std::cout << "[DEBUG] Max body size for chunked request: " << q->maxBodySize << std::endl;

					++i;
					return processChunkedData(buffer, n, q, onResponse, i, bDelete);
				}
				else
				{
					std::cout << "[DEBUG] No body, calling responseHook (bodySize=" << q->bodySize << ", chunkedEncoding=" << q->chunkedEncoding << ")" << std::endl;
					if (responseHook(q, onResponse))
						return (1);
				}
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