/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv3.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/20 13:10:03 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/27 11:14:05 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

bool Webserv::isRunnable(const std::string& file) const
{
    size_t  pos; 
	std::string ext;

	pos = file.find_last_of(".");
	if (pos != std::string::npos)
		ext = file.substr(pos + 1, file.length() - pos);
	if (ext == "py")
        return (true);
    return (false);
}

int Webserv::callCGI(const std::string& file, std::map<std::string, std::string>& env, CGI*& cgi) const
{
    std::string addDot;
    addDot = "." + file;
    cgi = NULL;

    try
    {
        cgi = new CGI(addDot, env);
    }
    catch(const std::exception& e)
    {
        return (1);
    }
    return (0);
}

bool Webserv::getCGI(int fd, CGI*& cgi) const
{
    const pollfd *fds;

    cgi = NULL;
    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (m_queries[i].cgi != NULL)
        {
            fds = m_queries[i].cgi->getPoll();
            pollfd (&arr)[2] = *reinterpret_cast<pollfd (*)[2]>(const_cast<pollfd *>(fds));
            if (arr[0].fd == fd || arr[1].fd == fd)
                return (cgi = m_queries[i].cgi, true);
        }
    }
    return (false);
}