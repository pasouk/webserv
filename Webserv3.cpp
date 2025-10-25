/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv3.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/20 13:10:03 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/25 14:10:49 by fabrice          ###   ########.fr       */
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