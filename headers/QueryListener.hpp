/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QueryListener.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 11:50:25 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/11/14 08:41:52 by fabrice          ###   ########.fr       */
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
# include "utils.hpp"

//extern bool g_listening;

class QueryListener
{
public:
	QueryListener(uint16_t, const std::string&);
	QueryListener();
	~QueryListener();
	QueryListener(const QueryListener&);

	QueryListener& operator=(const QueryListener&);

	int getListenFD() const;
	const struct sockaddr_in& getServerAddress() const;
	
private:
	sockaddr_in m_serverAddress;
	int			m_listenFD;

private:
	void initListener(uint16_t = 80, const std::string& = "0.0.0.0");
};

#endif