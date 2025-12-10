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

s_location* Webserv::getLocationFromServer(s_server& s, const std::string& path)
{
    size_t len;
    s_location* loc;

    loc = NULL;
    len = 0;
    for (size_t i = 0; i < s.locations.size(); ++i)
    {
        if (path.find(s.locations[i].concatOrReplace) != std::string::npos)
        {
            if (s.locations[i].concatOrReplace.length() > len)
            {
                len = path.find(s.locations[i].concatOrReplace);
                loc = &s.locations[i];
            }
        }
    }
    return (loc);
}

void Webserv::updatePathFromLocation(s_location& l, s_server& s, std::string& path) const
{
    size_t pos;
    std::string by;
    std::string fileLocation, pathLocation;
    glob_t g;
	std::ostringstream oss;

    fileLocation = getFilename(l.concatOrReplace.c_str());
    if (!fileLocation.empty())
        l.concatOrReplace = l.concatOrReplace.substr(0
            , l.concatOrReplace.length() - fileLocation.length());
    pos = path.find(l.concatOrReplace);
    if (pos != std::string::npos)
    {
        if (l.type == ROOT)
        {
            by = l.by;
            if (l.by[l.by.length() - 1] == '/')
                by = l.by.substr(0, l.by.length() - 1);
            pathLocation = by + l.concatOrReplace;
            path.replace(pos, l.concatOrReplace.size(), pathLocation);
        }
        else if (l.type == ALIAS)
        {
            by = l.by;
            if (l.by[l.by.length() - 1] != '/')
                by = l.by + "/";
            pathLocation = by;
            path.replace(pos, l.concatOrReplace.size(), pathLocation);
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
            size_t j;
            for (j = 0; j < g.gl_pathc; ++j)
            {
                if (path == g.gl_pathv[j])
                {
                    break ;
                }
            }
            /*if (j == g.gl_pathc)
            {
                oss << "[server] " << path << " doesn't exist";
                logErrMessage(oss);
                loc = NULL;
            }*/
        }
        globfree(&g);
    }
}

bool Webserv::isCgi(s_location* l, s_server& s, const std::string& httpMethodArg, std::string& path, std::string& binary)
{
    path = httpMethodArg;
    binary = "";

    if (l && l->cgi_pass != "none")
    {
        updatePathFromLocation(*l, s, path);
        if (!is_elf_binary(path.c_str()) || !is_macho_binary(path.c_str()))
            binary = l->cgi_pass;
        return (true);
    }
    return (false);
}

int Webserv::callCGI(const std::string& file, std::map<std::string, std::string>& env, s_query*& q, std::string& binary) const
{
	std::ostringstream oss;

    try
    {
        if (binary.empty())
        {
			oss << "[cgi] " << file << " is running";
            q->cgi = new CGI(file, env, q->fd);
        }
        else
        {
			oss << "[cgi] " << binary << " use " << file;
            q->cgi = new CGI(binary, file, env, q->fd);
        }
		logErrMessage(oss);
    }
    catch(const std::exception& e)
    {
        return (1);
    }
    return (0);
}

bool Webserv::getCgiQuery(int fd, s_query*& q)
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
 const s_http_path Webserv::parseHttpPath(s_location* l, s_server s, const std::string& path)
 {
    s_http_path httppath;
    size_t pos_s;
	std::string paths, buff;
    std::string::const_iterator c;

    httppath.path_updated = path;
    pos_s = path.find("?");
    if (pos_s != std::string::npos)
    { 
        httppath.query_string = path.substr(pos_s + 1, path.length() - pos_s + 1);
        httppath.path_updated = path.substr(0, pos_s);
    }
    paths = httppath.path_updated; 
    for (c = httppath.path_updated.end(); c != httppath.path_updated.begin(); --c)
    {
        if (*c == '/' || c == httppath.path_updated.end())
        {
            buff = paths;
            if (l)
                updatePathFromLocation(*l, s, buff);
            if (is_executable(buff.c_str()))
            {               
                httppath.path_info = path.substr(paths.length(), httppath.path_updated.length() - paths.length());
                httppath.path_updated = paths;
            }
        }
        if (!paths.empty())
            paths.erase(paths.size() - 1);
    }
    return (httppath);
}