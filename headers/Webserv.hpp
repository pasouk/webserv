/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:27:47 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/12/31 14:00:51 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "QueryListener.hpp"
# include <sys/resource.h>
# include <iomanip>
# include <algorithm>
# include <deque>
# include <map>
# include <ctime>
# include <glob.h>
# include "ContentLength.hpp"
# include "Chunked.hpp"

# define HEADER_BUFFER_SIZE 1024
# define BODY_BUFFER_SIZE 8192
# define KEEPALIVE_TIMEOUT 65
# define MAX_BODY_SIZE 8192

extern bool g_listening;

struct s_http_path
{
	s_http_path() : location(NULL) {}

	s_location* location;
	std::string	path_updated;
	std::string query_string;
	std::string path_info;
};

class Webserv
{
public:
	Webserv(ConfigParser*);
	Webserv();
	~Webserv();
	Webserv(const Webserv&);

	Webserv& operator=(const Webserv&);
	
	void startListening(void (*)(std::string&, CGI*, ParserHttpRequest&, s_server&));
	void printServers();
	void printQuery(s_query&) const;
	void cleanWebserv();

private:
	size_t m_keepalive_timeout;
	size_t m_client_buffers_size[2];	//0: header, 1: body
	std::vector<s_server> m_servers;		//servers list
	std::vector<s_query> m_clients;		//connected clients
	std::vector<s_query> m_queries;
	std::vector<pollfd> m_fds;
	std::vector<fdType> m_fdType;
	std::vector<const QueryListener*> m_listeners;

private:
	QueryListener* createListener(u_int16_t, const std::string&);
	std::vector<s_server> createServers(const ConfigParser*);
	void printServer(s_server&) const;
	void addClient(int);
	int readQuery(int, void (*)(std::string&, CGI*, ParserHttpRequest&, s_server&));
	int sendQuery(int);
	void stopListening();
	void destroyClient(int);
	void releaseQueries(int);
	int responseHook(s_query*&,  void (*)(std::string&, CGI*, ParserHttpRequest&, s_server&));
	int tcpStream(char* buffer, ssize_t, s_query*&
		, void (*)(std::string&, CGI*, ParserHttpRequest&, s_server&), bool&);
	bool clientNeedsAnswer(int) const;
	bool keepAlive(int, double);
	bool getClient(int, s_query*&);
	std::pair<char*, ssize_t> removeChunk(char*, ssize_t);
	const std::vector<std::string> getDiretiveValue(const Node*, const std::vector<const Node*>) const;
	s_server& getRightServer(s_query*&);
	bool matchServerName(const std::string&, const std::string&) const;
	int createCGI(const std::string&, std::map<std::string, std::string>&, s_query*&, std::string&);
	bool getCgiQuery(int, s_query*&);
	void updatePathAndLocation(s_location&, std::string&, const s_server&) const;
	const s_http_path& parseHttpPath(s_http_path&, s_server, std::string&);
	s_http_path getLocationFromServer(s_server&, const ParserHttpRequest&);
	TransferEncoding* bodyManagement(ssize_t&, const std::map<std::string, std::string>&);
};

#endif