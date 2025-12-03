/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv3.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/20 13:10:03 by fabrice           #+#    #+#             */
/*   Updated: 2025/11/23 16:15:11 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

const location* Webserv::buildPathFromLocation(server& s, std::string& path)
{
    size_t pos;
    std::string by;
    std::string fileLocation, pathLocation;
    location *loc;
    glob_t g;

    loc = NULL;
    for (size_t i = 0; i < s.locations.size(); ++i)
    {
        fileLocation = getFilename(s.locations[i].concatOrReplace.c_str());
        if (!fileLocation.empty())
            s.locations[i].concatOrReplace = s.locations[i].concatOrReplace.substr(0
                , s.locations[i].concatOrReplace.length() - fileLocation.length());
        pos = path.find(s.locations[i].concatOrReplace);
        if (pos != std::string::npos)
        {
            if (s.locations[i].type == ROOT)
            {
                by = s.locations[i].by;
                if (s.locations[i].by[s.locations[i].by.length() - 1] == '/')
                    by = s.locations[i].by.substr(0, s.locations[i].by.length() - 1);
                pathLocation = by + s.locations[i].concatOrReplace;
                path.replace(pos, s.locations[i].concatOrReplace.size(), pathLocation);
            }
            else if (s.locations[i].type == ALIAS)
            {
                by = s.locations[i].by;
                if (s.locations[i].by[s.locations[i].by.length() - 1] != '/')
                    by = s.locations[i].by + "/";
                pathLocation = by;
                path.replace(pos, s.locations[i].concatOrReplace.size(), pathLocation);
            }
            else
            {
                by = s.root;
                if (s.root[s.root.length() - 1] == '/')
                    by = s.root.substr(0, s.root.length() - 1);
                pathLocation = by;
                path = pathLocation + path;
            }
            if (!glob((pathLocation + fileLocation).c_str(), 0, NULL, &g))
            {
                for (size_t j = 0; j < g.gl_pathc; ++j)
                {
                    if (path == g.gl_pathv[j])
                    {
                        loc = &s.locations[i];
                        break ;
                    }
                }
            }
            globfree(&g);
            return (loc);
        }
    }
    return (NULL);
}

bool Webserv::isCgi(server& s, const std::string& httpMethodArg, std::string& path, std::string& interpreter)
{
    const location* l;

    path = httpMethodArg;
    interpreter = "";
    if ((l = buildPathFromLocation(s, path)) != NULL && l->cgi_pass != "none")
    {
        if (!is_elf_binary(path.c_str()) || !is_macho_binary(path.c_str()))
            interpreter = l->cgi_pass;
        return (true);
    }
    return (false);
}

int Webserv::callCGI(const std::string& file, std::map<std::string, std::string>& env, query*& q, std::string& interpreter) const
{
	std::ostringstream oss;

    try
    {
        if (interpreter.empty())
        {
			oss << "[cgi] " << file << " is running";
            q->cgi = new CGI(file, env, q->fd);
        }
        else
        {
			oss << "[cgi] " << interpreter << " makes " << file << " run" ;
            q->cgi = new CGI(interpreter, file, env, q->fd);
        }
		logErrMessage(oss);
    }
    catch(const std::exception& e)
    {
        return (1);
    }
    return (0);
}

bool Webserv::getCgiQuery(int fd, query*& q)
{
    const pollfd *fds;

    for (size_t i = 0; i < m_queries.size(); ++i)
    {
        if (m_queries[i].cgi != NULL)
        {
            fds = m_queries[i].cgi->getPollfd();
            pollfd (&arr)[2] = *reinterpret_cast<pollfd (*)[2]>(const_cast<pollfd *>(fds));
            if (arr[0].fd == fd || arr[1].fd == fd)
                return (q = &(m_queries[i]), true);
        }
    }
    return (false);
}