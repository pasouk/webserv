/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:27:47 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/11/23 16:15:11 by fabrice          ###   ########.fr       */
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
# include "ParserHttp.hpp"
# include "CGI.hpp"

# define HEADER_BUFFER_SIZE 1024
# define BODY_BUFFER_SIZE 8192
# define KEEPALIVE_TIMEOUT 65
# define MAX_BODY_SIZE 8192

extern bool g_listening;

struct s_location
{
public:
	bool					is_cgi;
	locationType			type;
	std::string				concatOrReplace;
	std::string				by;
	std::string				max_body_size;
	std::string				cgi_pass;
	std::string				index;
	std::vector<HttpMethod>	httpMethodsAllowed;
};

struct s_query
{	
	int							fd;
	time_t						lifeTime;
	size_t						byteSent;
	ssize_t						bodySize;
	uint16_t 					port;
	std::string					host;
	std::string					hostName;
	std::string					httpRequest;
	std::string 				formatedResponse;
	ParserHttpRequest*			httpParser;
	CGI*						cgi;
	std::deque<std::pair<char*, ssize_t> >	bodyChunks;
};

struct s_server
{
	std::vector<s_location>			locations;
	std::vector<std::string>		server_names;
	std::vector<uint16_t> 			ports;
	std::vector<std::string>		hosts;
	std::string						root;
	std::string						max_body_size;
};

struct s_http_path
{
	std::string location;
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
	bool isCgi(s_location*, s_server&, const std::string&, std::string&, std::string&);
	bool updatePathAndLocation(s_location&, std::string&, const s_server&) const ;
	const s_http_path parseHttpPath(s_location*, s_server, const std::string&);
	s_location* getLocationFromServer(s_server&, const ParserHttpRequest&);
};

#endif