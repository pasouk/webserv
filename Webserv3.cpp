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
#include <fnmatch.h>

s_http_path Webserv::getLocationFromServer(s_server& s, const ParserHttpRequest& http)
{
    size_t len;
    s_location l, m;
    std::string fileName, filePath, fileLoc, path;
    std::ostringstream oss;
    s_http_path httppath; 
    
    path = http.getPath();
    len = 0;
    std::cout << "[DEBUG getLocationFromServer] Looking for path: '" << path << "'" << std::endl;
    for (size_t i = 0; i < s.locations.size(); ++i)
    {
        l = s.locations[i];
        std::string originalConcat = l.concatOrReplace;
        fileName = getFilename(l.concatOrReplace.c_str());
        // Only remove filename if the location ends with '/' (it's a directory)
        // Don't remove if it's an exact match like '/post_body'
        if (!fileName.empty() && l.concatOrReplace[l.concatOrReplace.length() - 1] == '/')
            l.concatOrReplace = l.concatOrReplace.substr(0, l.concatOrReplace.length() - fileName.length());
        std::cout << "[DEBUG] Location " << i << ": original='" << originalConcat << "', after='" << l.concatOrReplace << "', fileName='" << fileName << "'" << std::endl;
        if (path.find(l.concatOrReplace) != std::string::npos)
        {
            if (l.concatOrReplace.length() > len)
            {
                len = l.concatOrReplace.length();
                httppath.location = &s.locations[i]; 
            }
        }
    }
    //std::cout << "BEFORE:" << path << std::endl;
    updatePathAndLocation(l, path, s);
    //std::cout << "AFTER: " << path << std::endl;
    httppath = parseHttpPath(/*loc*/httppath, s, path);
    //std::cout << "PATH INFO: " << httppath.path_info << std::endl;
    //std::cout << "PATH UPDA: " << httppath.path_updated << std::endl;
    //std::cout << "QUERY STR: " << httppath.query_string << std::endl;
    for (size_t j = 0; j < s.locations.size(); ++j)
    {
        m = s.locations[j];
        fileName = getFilename(httppath.path_updated);
        fileLoc = getFilename(m.concatOrReplace.c_str());
        filePath = httppath.path_updated.substr(0, httppath.path_updated.length() - fileName.length());
        if (!fileName.empty() && !fileLoc.empty())
        {
            //std::cout << "CHECK: " << filePath << " --> " << fileName << std::endl;
            //std::cout << " WITH: " << filePath << " --> " << fileLoc << std::endl;

            // Only use fnmatch if the location pattern contains wildcards
            // This prevents matching non-wildcard paths like /post_body
            if (fileLoc.find('*') != std::string::npos ||
                fileLoc.find('?') != std::string::npos ||
                fileLoc.find('[') != std::string::npos)
            {
                // Use fnmatch to check if the filename matches the pattern
                // This works even if the file doesn't exist yet (important for CGI)
                if (fnmatch(fileLoc.c_str(), fileName.c_str(), 0) == 0)
                {
                    std::cout << "PATTERN MATCH: " << fileName << " matches pattern " << fileLoc << std::endl;
                    httppath.location = &s.locations[j];
                    break;
                }
            }
        }
    }
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

/*bool*/void Webserv::updatePathAndLocation(s_location& l, std::string& path, const s_server& s) const
{
    size_t pos;
    std::string by;
    std::string fileName, pathLocation;

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
    }
}

int Webserv::createCGI(const std::string& file, std::map<std::string, std::string>& env, s_query*& q, std::string& binary)
{
	std::ostringstream oss;

	std::cout << "[CREATE CGI] bodyFileFd=" << q->bodyFileFd << std::endl;

    try
    {
        if (binary.empty())
        {
            q->cgi = new CGI(file, env, q->fd, m_fds, m_fdType, q->bodyFileFd);
			oss << "client fd:" << q->fd << ", " << file << " is build with bodyFileFd=" << q->bodyFileFd;
        }
        else
        {
            q->cgi = new CGI(binary, file, env, q->fd, m_fds, m_fdType, q->bodyFileFd);
			oss << "client fd:" << q->fd << ", " << binary << " use " << file << " as argument with bodyFileFd=" << q->bodyFileFd;
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
const s_http_path& Webserv::parseHttpPath(s_http_path& httpPath, s_server s, std::string& path)
{
    //s_http_path httpPath;
    size_t pos_s;
	std::string paths, pathNotRef;
    std::string::const_iterator c;
    s_location lNotRef; 

    httpPath.path_updated = path;
    //httpPath.location = l;
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
            if (httpPath.location)
            {
                lNotRef = *httpPath.location;
                updatePathAndLocation(lNotRef, pathNotRef, s);
            }
            if (is_executable(pathNotRef.c_str()))
            {               
                httpPath.path_info = path.substr(paths.length(), httpPath.path_updated.length() - paths.length());
                httpPath.path_updated = pathNotRef;
            }
        }
        if (!paths.empty())
            paths.erase(paths.size() - 1);
    }
    return (httpPath);
}