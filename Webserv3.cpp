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

s_location* Webserv::getLocationFromServer(s_server& s, const ParserHttpRequest& http)
{
    size_t len;
    s_location  *loc, l;
    std::string fileName, path;

    path = http.getPath();
    loc = NULL;
    len = 0;
    for (size_t i = 0; i < s.locations.size(); ++i)
    {
        l = s.locations[i];
        fileName = getFilename(l.concatOrReplace.c_str());
        if (!fileName.empty())
            l.concatOrReplace = l.concatOrReplace.substr(0, l.concatOrReplace.length() - fileName.length());
        if (path.find(l.concatOrReplace) != std::string::npos)
        {
            if (l.concatOrReplace.length() > len)
            {
                len = l.concatOrReplace.length();
                loc = &s.locations[i];
            }
        }
    }
    if (loc)
    {
        for (size_t i = 0; i < loc->httpMethodsAllowed.size(); ++i)
        {
            if (loc->httpMethodsAllowed[i] == http.getMethod())
                return (loc);
        }
        if (loc->httpMethodsAllowed.size())
            return (NULL);
    }  
    return (loc);
}

bool Webserv::updatePathAndLocation(s_location& l, std::string& path, const s_server& s) const
{
    size_t pos, j;
    std::string by;
    std::string fileName, pathLocation;
    glob_t g;
    bool bValidPath;

    bValidPath = true;
    fileName = getFilename(l.concatOrReplace.c_str());
    if (!fileName.empty())
        l.concatOrReplace = l.concatOrReplace.substr(0, l.concatOrReplace.length() - fileName.length());
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
        if (!fileName.empty())
        {
            bValidPath = false;
            if (!glob((pathLocation + fileName).c_str(), 0, NULL, &g))
            {
                for (j = 0; j < g.gl_pathc; ++j)
                    if (std::string(g.gl_pathv[j]).find(path) != std::string::npos)
                    {
                        bValidPath = true;
                        break ;
                    }
            }
            globfree(&g);
        }
    }
    return (bValidPath);
}

bool Webserv::isCgi(s_location* l, s_server& s, const std::string& httpMethodArg, std::string& path, std::string& binary)
{
    std::ostringstream oss;

    path = httpMethodArg;
    binary = "";
    if (l && l->is_cgi)
    {
        if (!updatePathAndLocation(*l, path, s))
        {
            oss << path << " not found\n";
            logErrMessage(oss);
            return (false);
        }      
        binary = l->cgi_pass;
        return (true);
    }
    return (false);
}

int Webserv::createCGI(const std::string& file, std::map<std::string, std::string>& env, s_query*& q, std::string& binary)
{
	std::ostringstream oss;

    try
    {
        if (binary.empty())
        {
            q->cgi = new CGI(file, env, q->fd, m_fds, m_fdType);
			oss << "client fd:" << q->fd << ", " << file << " is build";
        }
        else
        {
            q->cgi = new CGI(binary, file, env, q->fd, m_fds, m_fdType);
			oss << "client fd:" << q->fd << ", " << binary << " use " << file << " as argument";
        }
		logOutMessage(oss);
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
    s_http_path httpPath;
    size_t pos_s;
	std::string paths, pathNotRef;
    std::string::const_iterator c;
    s_location lNotRef;
       
    httpPath.path_updated = path;
    pos_s = path.find("?");
    if (pos_s != std::string::npos)
    { 
        httpPath.query_string = path.substr(pos_s + 1, path.length() - pos_s + 1);
        httpPath.path_updated = path.substr(0, pos_s);
    }
    paths = httpPath.path_updated;
    for (c = httpPath.path_updated.end(); c != httpPath.path_updated.begin(); --c)
    {
        if (*c == '/' || c == httpPath.path_updated.end())
        {
            pathNotRef = paths;
            if (l)
            {
                lNotRef = *l;
                updatePathAndLocation(lNotRef, pathNotRef, s);
            }
            if (is_executable(pathNotRef.c_str()))
            {               
                httpPath.path_info = path.substr(paths.length(), httpPath.path_updated.length() - paths.length());
                httpPath.path_updated = paths;
            }
        }
        if (!paths.empty())
            paths.erase(paths.size() - 1);
    }
    return (httpPath);
}