/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:27:47 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/16 10:16:28 by fbuyl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "QueryListener.hpp"
# include <sys/resource.h>
# include <iomanip>
# include <algorithm>

# define BODY_BUFFER_SIZE 4096

extern bool g_listening;

struct query
{
	int							fd;
	size_t						bodySize;
	uint16_t 					port;
	std::string					host;
	std::string					httpRequest;
	std::string					httpBody;
	//ParerHttpRequest			parsedRequest
	//std::string 				formatedResponse
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
	
	void startListening(void (*)(query&, Webserv*)
		, void (*)(std::vector<query>&, std::vector<server>&, Webserv*));
	void printServers();
	void printQuery(query&) const;

private:
	size_t m_body_buffer_size;
	ConfigParser* m_parser;
	std::vector<query> m_queries;		//list of all waiting queries
	std::vector<server> m_servers;		//list of all listening servers
	std::vector<query> m_clients;
	std::vector<pollfd> m_fds;
	std::vector<bool> m_isClient;
	std::vector<const QueryListener*> m_listeners;

private:
	QueryListener* createListener(u_int16_t, const std::string&);
	std::vector<server> findServers() const;
	void cleanWebserv();
	void printServer(server&) const;
	void addClient(size_t);
	void readQuery(size_t, void (*)(query&, Webserv*)
		, void (*)(std::vector<query>&, std::vector<server>&, Webserv*));
	void sendQuery(size_t);
	void stopListening();
	void tcpStream(char* buffer, size_t, std::vector<query>::iterator
		, void (*)(query&, Webserv*));
};

#endif