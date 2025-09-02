/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:27:47 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/31 14:51:05 by fbuyl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "QueryListener.hpp"
# include <sys/resource.h>
# include <algorithm>

# define BUFFER_SIZE 4096

extern bool g_listening;

class Webserv
{
public:
	Webserv(ConfigParser*);
	Webserv();
	~Webserv();
	Webserv(const Webserv&);

	Webserv& operator=(const Webserv&);
	void queriesListen();
	void stopListening();

private:
	std::vector<pollfd> m_fds;
	std::vector<bool> m_isClient;
	std::vector<QueryListener*> m_listeners;

private:
	void cleanWebserv();
};

#endif