/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv3.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/20 13:10:03 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/24 15:01:42 by fabrice          ###   ########.fr       */
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

int Webserv::callCGI(const ParserHttpRequest* http, CGI*& cgi)
{
    std::string file = http->getPath();
    file = "." + file;
    if (isRunnable(file))
    {
        try
        {
            std::map<std::string, std::string> env;
            env["QUERY_STRING"] = "test";
            env["PATH_INFO"] = "test2";
            cgi = new CGI(file, env);
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