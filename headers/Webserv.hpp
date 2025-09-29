/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:27:47 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/29 12:56:29 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "QueryListener.hpp"
# include <sys/resource.h>
# include <iomanip>
# include <algorithm>
# include <deque>
# include <ctime>

# define HEADER_BUFFER_SIZE 1024
# define BODY_BUFFER_SIZE 8192
# define KEEPALIVE_TIMEOUT 65

extern bool g_listening;

struct query
{	
	int							fd;
	time_t						lifeTime;
	size_t						byteSent;
	ssize_t						bodySize;
	uint16_t 					port;
	std::string					host;
	std::string					httpRequest;
	std::deque<char*>			bodyChunks;
	std::string 				formatedResponse;
};

struct server
{
	std::vector<std::string>	server_names;
	std::vector<uint16_t> 		ports;
	std::vector<std::string>	hosts;
	std::string					root;	
};

class Webserv
{
public:
	Webserv(ConfigParser*);
	Webserv();
	~Webserv();
	Webserv(const Webserv&);

	Webserv& operator=(const Webserv&);
	
	void startListening(void (*)(query&, std::vector<server>&, Webserv*));
	void printServers();
	void printQuery(query&) const;

private:
	ConfigParser* m_parser;
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
	std::vector<server> createServers() const;
	void cleanWebserv();
	void printServer(server&) const;
	void addClient(size_t);
	void readQuery(size_t, void (*)(query&, std::vector<server>&, Webserv*));
	void sendQuery(size_t);
	void stopListening();
	void destroyClient(size_t);
	void destroyClientQueries(size_t);
	void queryHook(std::vector<query>::iterator,  void (*)(query&, std::vector<server>&, Webserv*));
	char* removeChunk(char*, ssize_t);
	bool tcpStream(char* buffer, ssize_t, std::vector<query>::iterator, void (*)(query&, std::vector<server>&, Webserv*));
	bool needAResponse(size_t) const;
	bool keepAlive(size_t, double) const;
	bool getClient(size_t, query&) const;
	ssize_t checkForContentLength(query&) const;
};

#endif