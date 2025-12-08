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

const s_location* Webserv::buildPathFromLocation(s_server& s, std::string& path) const
{
    size_t pos;
    std::string by;
    std::string fileLocation, pathLocation;
    s_location *loc;
    glob_t g;
	std::ostringstream oss;

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
                loc = &s.locations[i];
            }
            else if (s.locations[i].type == ALIAS)
            {
                by = s.locations[i].by;
                if (s.locations[i].by[s.locations[i].by.length() - 1] != '/')
                    by = s.locations[i].by + "/";
                pathLocation = by;
                path.replace(pos, s.locations[i].concatOrReplace.size(), pathLocation);
                loc = &s.locations[i];
            }
            else
            {
                by = s.root;
                if (s.root[s.root.length() - 1] == '/')
                    by = s.root.substr(0, s.root.length() - 1);
                pathLocation = by;
                path = pathLocation + path;
                loc = &s.locations[i];
            }
            if (!glob((pathLocation + fileLocation).c_str(), 0, NULL, &g))
            {
                size_t j;
                for (j = 0; j < g.gl_pathc; ++j)
                {
                    if (path == g.gl_pathv[j])
                    {
                        loc = &s.locations[i];
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
            return (loc);
        }
    }
    return (NULL);
}

bool Webserv::isCgi(s_server& s, const std::string& httpMethodArg, std::string& path, std::string& binary)
{
    const s_location* l;

    path = httpMethodArg;
    binary = "";


    if ((l = buildPathFromLocation(s, path)) != NULL && l->cgi_pass != "none")
    {
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
			oss << "[cgi] " << binary << " use " << file << " as argument" ;
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
 const s_http_path Webserv::parseHttpPath(s_server s, const std::string& path)
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
            buildPathFromLocation(s, buff);
            if (is_executable(buff.c_str()))
            {
                httppath.path_info = path.substr(paths.length(), httppath.path_updated.length() - paths.length());
                httppath.path_updated = paths;
            }
        }
        if (!paths.empty())
            paths.erase(paths.size() - 1);
    }
    std::cerr << "PATH: " <<  httppath.path_updated << std::endl;
    std::cerr << "INFO: " << httppath.path_info << std::endl;
    std::cerr << "STRI: " <<httppath.query_string << std::endl;
    return (httppath);
}