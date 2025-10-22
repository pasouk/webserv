/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv3.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/20 13:10:03 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/22 13:02:23 by fabrice          ###   ########.fr       */
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
	if (ext == "cgi")
        return (true);
    return (false);
}

int Webserv::callCGI(const std::string& file, CGI*& cgi)
{
    if (isRunnable(file))
    {
        try
        {
            cgi = new CGI(file);
        }
        catch(const std::exception& e)
        {
 		    return (1);
        }
    }
    else
        cgi = NULL;
    return (0);
}