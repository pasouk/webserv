/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:27:47 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/06 13:27:05 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "QueryListener.hpp"
# include <sys/resource.h>
# include <iomanip>
# include <algorithm>

# define BUFFER_SIZE 4096

extern bool g_listening;

struct query
{
	int							fd;
	uint16_t 					port;
	std::string					host;
	std::string					http;
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
	
	void startListening();
	void stopListening();
	const std::vector<query>& getQueries() const;	//get a list of all waiting queries
	const std::vector<server>& getServers() const;	//get e list of all listening servers
	void printServers();

private:
	ConfigParser* m_parser;
	std::vector<query> m_queries;
	std::vector<server> m_servers;
	std::vector<query> m_clients;
	std::vector<pollfd> m_fds;
	std::vector<bool> m_isClient;
	std::vector<const QueryListener*> m_listeners;

private:
	void cleanWebserv();
	QueryListener* createListener(u_int16_t, const std::string&);
	void printQuery(query&) const;
	void printServer(server&) const;
	void addClient(size_t);
	void checkQueries(size_t);
	std::vector<server> findServers() const;
};

#endif