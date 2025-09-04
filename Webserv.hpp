/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:27:47 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/04 15:51:51 by fabricebuyl      ###   ########.fr       */
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

struct Query
{
	int							fd;
	std::vector<std::string> 	server_names;
	uint16_t 					port;
	std::string					host;
	std::string					root;
	std::string					http;
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

private:
	ConfigParser* m_parser;
	std::vector<Query> m_queries;
	std::vector<pollfd> m_fds;
	std::vector<bool> m_isClient;
	std::vector<const QueryListener*> m_listeners;
	std::vector<const Node*> m_server_names;
	std::vector<const Node*> m_roots;
	std::vector<const Node*> m_locations;

private:
	void cleanWebserv();
	QueryListener* createListener(u_int16_t, const std::string&);
	void printQuery(Query&) const;
	void addClient(size_t);
	void checkQueries(size_t);
	std::vector<std::string> getArgsFromServerDirective(const std::string&, uint16_t, const std::string&) const;
};

#endif