/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv3.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/20 13:10:03 by fabrice           #+#    #+#             */
/*   Updated: 2026/02/14 14:06:25 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "CGI.hpp"

s_http_path Webserv::getLocationFromServer(s_server& s, const ParserHttpRequest& http)
{
    size_t len;
    size_t pos_s;
    s_location l, m;
    std::string fileName, filePath, fileLoc, path, part, path_updated;
    std::string::const_iterator c;
    std::ostringstream oss;
    s_http_path httppath;
    bool bFileLoc;
    //size_t k= -1;
    
    path = http.getPath();
    pos_s = path.find("?");
    if (pos_s != std::string::npos)
        httppath.query_string = path.substr(pos_s + 1, path.length() - pos_s + 1);
    httppath.path_updated = path.substr(0, pos_s);
    path = httppath.path_updated;
    len = 0;
    bFileLoc = false;
    for (size_t i = 0; i < s.locations.size(); ++i)
    { 
        part = path;
        for (int j = part.length() - 1; j >= 0; --j)
        {
            if (part[j] == '/' || !j)
            {
                l = s.locations[i];
                fileName = getFilename(l.concatOrReplace.c_str());
                //if (part == path)
                //    std::cout << "LOCATION " << i << ": " << l.concatOrReplace << std::endl;
                //std::cout << "PART: " << part << std::endl;
                if (fileName.empty())
                {
                    if (part == path && l.concatOrReplace.length() > len)
                    {
                        //std::cout <<httppath.path_updated << std::endl;
                        path_updated = httppath.path_updated;
                        len = l.concatOrReplace.length();
                        httppath.location = &s.locations[i];
                        //k = i;
                        updatePathAndLocation(s.locations[i], path_updated, s);
                        bFileLoc = true;
                    }
                }
                else
                {
                    if (fnmatch(l.concatOrReplace.c_str(), part.c_str(), FNM_PATHNAME) == 0)
                    {
                        //std::cout << part << " :: " << path << std::endl;
                        httppath.path_info = path.substr(part.length(), path.length() - part.length());
                        httppath.location = &s.locations[i];
                        path_updated = httppath.path_updated.substr(0, httppath.path_updated.length() - httppath.path_info.length());
                        //k = i;
                        updatePathAndLocation(s.locations[i], path_updated, s);
                        bFileLoc = false;
                    }
                }
                part = path.substr(0, j);
            }
        }
    }
    httppath.path_updated = path_updated;
    if (bFileLoc && httppath.location)
    {
        httppath.location = NULL;
        for (size_t i = 0; i < s.locations.size() && !httppath.location; ++i)
        {
            part = path;
            httppath.path_info = "";
            for (int j = part.length() - 1; j >= 0; --j)
            {
                if (part[j] == '/' || !j)
                {
                    fileName = part;
                    part = path.substr(0, j);
                    fileName = fileName.substr(part.size() + 1);
                    if (fnmatch(s.locations[i].concatOrReplace.c_str(), fileName.c_str(), FNM_PATHNAME) == 0)
                    {
                        httppath.location = &s.locations[i];
                        httppath.path_updated = httppath.path_updated.substr(0,
                            httppath.path_updated.length() - httppath.path_info.length());
                        break ;
                    }
                    else
                        httppath.path_info.insert(0, "/" + fileName);
                }
            }
        }
    }
    //std::cout << "INDEX LOC: " << k << std::endl;
    //std::cout << "QUERY STRING: " << httppath.query_string << std::endl;
    //std::cout << "PATH UPDATED: " << httppath.path_updated << std::endl;
    //std::cout << "PATH INFO:    " << httppath.path_info << std::endl;
    if (httppath.location)
    {
        for (size_t i = 0; i < httppath.location->httpMethodsAllowed.size(); ++i)
        {
            if (httppath.location->httpMethodsAllowed[i] == http.getMethod())
                return (httppath);
        }
        if (httppath.location->httpMethodsAllowed.size())
        {
			oss << "method: " << methods_map[http.getMethod()].name << " not allowed with "
                << httppath.location->concatOrReplace << " location";
		    logOutMessage(oss);
            httppath.location = NULL;
        }
    }  
    return (httppath);
}

/*bool*/void Webserv::updatePathAndLocation(s_location& loc, std::string& path, const s_server& s) const
{
    size_t pos;
    std::string by;
    std::string fileName, pathLocation;

    fileName = getFilename(loc.concatOrReplace.c_str());
    if (!fileName.empty())
        loc.concatOrReplace = loc.concatOrReplace.substr(0, loc.concatOrReplace.length() - fileName.length());
    pos = path.find(loc.concatOrReplace);
    if (pos != std::string::npos)
    {
        if (loc.type == LOCATION_ROOT)
        {
            by = loc.by;
            if (loc.by[loc.by.length() - 1] == '/')
                by = loc.by.substr(0, loc.by.length() - 1);
            pathLocation = by + loc.concatOrReplace;
            path.replace(pos, loc.concatOrReplace.size(), pathLocation);

        }
        else if (loc.type == LOCATION_ALIAS)
        {            
            by = loc.by;
            if (loc.by[loc.by.length() - 1] != '/')
                by = loc.by + "/";
            pathLocation = by;
            path.replace(pos, loc.concatOrReplace.size(), pathLocation);
        }
        else
        {
            by = s.root;
            if (s.root[s.root.length() - 1] == '/')
                by = s.root.substr(0, s.root.length() - 1);
            pathLocation = by;
            path = pathLocation + path;
        }
    }
}

int Webserv::createCGI(const std::string& file, std::map<std::string, std::string>& env, s_query*& q, std::string& binary)
{
	std::ostringstream oss;

    try
    {
        if (binary.empty())
        {
            q->cgi = new CGI(file, env, q, m_fds, m_fdType);
			oss << "client fd:" << q->fd << ", " << file << " is build";
        }
        else
        {
            q->cgi = new CGI(binary, file, env, q, m_fds, m_fdType);
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

TransferEncoding* Webserv::bodyManagement(ssize_t& bodySize, const std::map<std::string, std::string>& headers)
{
    TransferEncoding* te;
    std::map<std::string, std::string> heds = headers;
	std::stringstream ss;
    std::string header;

    te = NULL;
    bodySize = 0;
    header = heds["Transfer-Encoding"];
    if (header.empty())
    {
		header = heds["Content-Length"];
        if (!header.empty())
        {
            ss.clear();
            ss << header;
            ss >> bodySize;
        }
        te = new (std::nothrow)ContentLength(m_client_buffers_size[1], m_client_body_temp_path);
    }
    else
        if (header == "chunked")
            te = new (std::nothrow)Chunked(m_client_buffers_size[1], m_client_body_temp_path);
    return (te);
}

void Webserv::addEnvMetaVariables(const std::map<std::string, std::string>& http, std::map<std::string, std::string>& env)
{
    (void)env;
    std::string meta;
    std::map<std::string, std::string> _http = http;

    for (std::map<std::string, std::string>::const_iterator it = http.begin(); it != http.end(); ++it)
        if ((*it).first.find("X-") != std::string::npos)
        {
            meta = (*it).first;
            meta = meta.insert(0, "HTTP_");
            for (size_t i = 0; i < meta.size(); ++i)
                if (meta[i] == '-')
                    meta[i] = '_';
                else
                    meta[i] = std::toupper(static_cast<unsigned char>(meta[i]));
            env[meta] = (*it).second;
        } 
}