/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QueryListener.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 11:50:25 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/29 10:20:30 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUERYLISTENER_HPP
# define QUERYLISTENER_HPP

# include <sys/socket.h>
# include <iostream>
# include <unistd.h>
# include <netinet/in.h>
# include <fcntl.h>
# include <errno.h>
# include <cstring>
# include <signal.h>
# include <sys/wait.h>
# include <sys/mman.h>
# include <poll.h>
# include <arpa/inet.h>
# include "ConfigParser.hpp"

# define BUFFER_SIZE 4096
# define MAX_CLIENTS 10

extern bool g_listening;

class QueryListener
{
public:
	QueryListener(uint16_t, const std::string&);
	QueryListener();
	~QueryListener();
	QueryListener(const QueryListener&);

	QueryListener& operator=(const QueryListener&);
	
private:
	struct sockaddr_in 	m_serverAddress;
	struct pollfd 		m_fds[MAX_CLIENTS + 1];

private:
	void initListener(uint16_t = 80, const std::string& = "0.0.0.0");
	void queriesListen();
	void stopListening();
	void closeFds();
};

#endif