/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv3.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/20 13:10:03 by fabrice           #+#    #+#             */
/*   Updated: 2025/11/17 15:56:34 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

const location* Webserv::isThereLocation(const server& s, std::string& path)
{
    size_t pos;

    for (size_t i = 0; i < s.locations.size(); ++i)
    {
        pos = path.find(s.locations[i].concatOrReplace);
        if (pos != std::string::npos)
        {
            if (s.locations[i].type == ROOT)
                path.replace(pos, s.locations[i].concatOrReplace.size()
                    , s.locations[i].by + s.locations[i].concatOrReplace);
            else if (s.locations[i].type == ALIAS)
                path.replace(pos, s.locations[i].concatOrReplace.size(), s.locations[i].by);
            if (RELATIVE)
                if (path[0] == '/')
                    path = path.substr(1, path.length() - 1);
            return (&s.locations[i]);
        }
    }
    return (NULL);
}

bool Webserv::isCgi(const server& s, const std::string& httpMethodArg, std::string& path, bool& binary)
{
    const location* l;

    path = httpMethodArg;
    binary = false;
    if ((l = isThereLocation(s, path)) != NULL && l->cgi_pass != "none")
    {
        path = l->cgi_pass + getFilename(httpMethodArg);
        if (is_elf_binary(l->cgi_pass.c_str()) || is_macho_binary(l->cgi_pass.c_str()))
        {
            path = l->cgi_pass;
            binary = true;
        }
        return (true);
    }
    return (false);
}

int Webserv::callCGI(const std::string& file, std::map<std::string, std::string>& env, query& q, bool isBinary) const
{
    std::string addDot;

    try
    {
        if (isBinary)
        {
            std::cout << "IS BINARY\n";
            addDot = "/home/fabrice/Documents/webserv/cgi-bin/hello.py"; //+ getFilename(q.httpParser->getPath());  //"." + q.httpParser->getPath();
            q.cgi = new CGI(file, addDot, env, q.fd);
        }
        else
        {
            std::cout << "IS NOT BINARY\n";
            addDot = "." + file;
            q.cgi = new CGI(addDot, env, q.fd);
        }
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