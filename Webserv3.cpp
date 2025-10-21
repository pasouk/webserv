/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv3.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/20 13:10:03 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/21 10:53:12 by fbuyl            ###   ########.fr       */
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
	{
		std::cout << "HELLO CGI !!\n";
        return (true);
	}
    return (false);
}

int Webserv::callCGI(const std::string& file, CGI*& cgi)
{
    if (isRunnable(file))
    {
        try
        {
            cgi = new CGI(file, this);
        }
        catch(const std::exception& e)
        {
 		    return (1);
        }
    }
    return (0);
}