/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:27:47 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/10/21 12:03:37 by fbuyl            ###   ########.fr       */
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
# include "ParserHttp.hpp"
# include "CGI.hpp"

# ifndef RELATIVE
#  define RELATIVE true
# endif
# define HEADER_BUFFER_SIZE 1024
# define BODY_BUFFER_SIZE 8192
# define KEEPALIVE_TIMEOUT 65

extern bool g_listening;

enum locationType
{
    ROOT,
    ALIAS,
	PROXY_PASS
};

struct location
{
public:
	std::string		concatOrReplace;
	locationType	type;
	std::string		by;
};

struct query
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

struct server
{
	std::vector<location>			locations;
	std::vector<HttpMethod>			httpMethodsAllowed;
	std::vector<std::string>		server_names;
	std::vector<uint16_t> 			ports;
	std::vector<std::string>		hosts;
	std::string						root;	
};

class Webserv
{
public:
	Webserv(ConfigParser*);
	Webserv();
	~Webserv();
	Webserv(const Webserv&);

	Webserv& operator=(const Webserv&);
	
	void startListening(void (*)(std::string&, ParserHttpRequest&, server&));
	void printServers();
	void printQuery(query&) const;
	void cleanWebserv();

private:
	size_t m_keepalive_timeout;
	size_t m_client_buffers_size[2];	//0: header, 1: body
	std::vector<server> m_servers;		//servers list
	std::vector<query> m_clients;		//connected clients
	std::vector<query> m_queries;
	std::vector<pollfd> m_fds;
	std::vector<bool> m_isClient;
	std::vector<const QueryListener*> m_listeners;

private:
	QueryListener* createListener(u_int16_t, const std::string&);
	std::vector<server> createServers(const ConfigParser*);
	void printServer(server&) const;
	void addClient(size_t);
	int readQuery(size_t, void (*)(std::string&, ParserHttpRequest&, server&));
	void sendQuery(size_t);
	void stopListening();
	void destroyClient(size_t);
	void destroyClientQueries(size_t);
	void responseHook(std::vector<query>::iterator,  void (*)(std::string&, ParserHttpRequest&, server&));
	int tcpStream(char* buffer, ssize_t, std::vector<query>::iterator
		, void (*)(std::string&, ParserHttpRequest&, server&), bool&);
	bool clientNeedsAnswer(size_t) const;
	bool keepAlive(size_t, double) const;
	bool getClient(size_t, query&) const;
	std::pair<char*, ssize_t> removeChunk(char*, ssize_t);
	const std::vector<std::string> getRoot(const Node*, const std::vector<const Node*>) const;
	server& getRightServer(query&);
	bool matchServerName(const std::string&, const std::string&) const;
	void addPipeToPoll(pollfd(&)[2]);
	void removePipeFromPoll(pollfd(&)[2]);
	int callCGI(const std::string&, CGI*&);
	bool isRunnable(const std::string&) const;
};

#endif