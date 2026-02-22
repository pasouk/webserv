/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 09:26:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2026/02/21 17:47:47 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "ParserHttp.hpp"
#include "HTTP_response/HttpResponse.hpp"

bool g_listening = true;

void handle_sigint(int sig)
{
	(void)sig;
	g_listening = false;
}

void resolvePath(ParserHttpRequest& r, s_server& s, std::string& root, std::string& mappedPath, s_location& foundLocation)
{
    mappedPath = r.getPath();
    root = s.root;
    //std::cout << "[DEBUG resolvePath] Initial - path='" << mappedPath << "' root='" << root << "'" << std::endl;

    if (s.locations.empty())
    {
        //std::cout << "[DEBUG resolvePath] No locations, returning with root='" << root << "'" << std::endl;
        return;
    }

    // --------------------------
    // 1) Find LONGEST matching prefix
    // --------------------------
    int bestIndex = -1;
    size_t bestLen = 0;

    for (size_t i = 0; i < s.locations.size(); ++i)
    {
        const std::string &prefix = s.locations[i].concatOrReplace;

        if (mappedPath.compare(0, prefix.size(), prefix) == 0)
        {
            if (prefix.size() > bestLen)
            {
                bestLen = prefix.size();
                bestIndex = (int)i;
            }
        }
    }

    if (bestIndex == -1)
    {
        //std::cout << "[DEBUG resolvePath] No matching location, using default root='" << root << "'" << std::endl;
        foundLocation = s_location();  // Initialize with empty location
        return;
    }

    const s_location &loc = s.locations[bestIndex];
    foundLocation = loc;  // Store the found location
    //std::cout << "[DEBUG resolvePath] Matched location " << bestIndex << " with prefix='" << loc.concatOrReplace << "'" << std::endl;

    // Remainder after removing prefix
    size_t prefix_len = loc.concatOrReplace.size();
    std::string remainder;

    if (mappedPath.size() > prefix_len)
        remainder = mappedPath.substr(prefix_len);

    // --------------------------
    // 2) TYPE ROOT
    // --------------------------
    if (loc.type == LOCATION_ROOT)
    {
        root = loc.by;
        //std::cout << "[DEBUG resolvePath] ROOT location - new root='" << root << "'" << std::endl;

        std::string newReqPath = remainder;

        if (newReqPath.empty())
            newReqPath = "/";
        else if (newReqPath[0] != '/')
            newReqPath = "/" + newReqPath;

        //std::cout << "[DEBUG resolvePath] ROOT location - new path='" << newReqPath << "'" << std::endl;
        r.setPath(newReqPath);
        mappedPath = newReqPath;
    }

    // --------------------------
    // 3) TYPE ALIAS
    // --------------------------
    else if (loc.type == LOCATION_ALIAS)
    {
        std::string newReqPath = loc.by;
        
        // Concatenate remainder to alias path
        if (!remainder.empty())
        {
            // Ensure proper path separator
            if (!newReqPath.empty() && newReqPath[newReqPath.size()-1] != '/')
                newReqPath += "/";
            if (!remainder.empty() && remainder[0] == '/')
                remainder = remainder.substr(1);
            newReqPath += remainder;
        }
        
        // With alias, the path IS the complete path (no root concatenation)
        root = "";
        
        //std::cout << "[DEBUG resolvePath] ALIAS location - new path='" << newReqPath << "' root='" << root << "'" << std::endl;

        r.setPath(newReqPath);
        mappedPath = newReqPath;
    }
    //std::cout << "[DEBUG resolvePath] Final - path='" << mappedPath << "' root='" << root << "'" << std::endl;
}

void onResponse(std::string& response, CGI* cgi, ParserHttpRequest& r, s_server& s)
{	
	(void)r;
	(void)s;
	std::ostringstream oss;
	std::stringstream ss;
	std::string path;
	std::string root;
	std::string body;
	s_location foundLoc;

	if (cgi)
	{
		if (cgi->wasExecuted() == false)
		{
			if (cgi->runCGI())
			{
				oss << "client fd:" << cgi->getFd() << ", cgi failed to run";
				logErrMessage(oss);
			}
		}
		else
		{
			ss << cgi->getResponse().length();
			body = cgi->getResponse().substr(cgi->getResponse().length() - cgi->getTotal(), cgi->getTotal());
			std::string responseBuild =
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: " + ss.str() + "\r\n"
				"\r\n"
				+ cgi->getResponse();
			response = responseBuild;
		}
	}
	else
	{
		// Check if we need to redirect to add trailing slash for directories
		std::string requestedPath = r.getPath();
		if (!requestedPath.empty() && requestedPath[requestedPath.size() - 1] != '/')
		{
			// Check if there's a location with trailing slash
			std::string pathWithSlash = requestedPath + "/";
			for (size_t i = 0; i < s.locations.size(); ++i)
			{
				if (s.locations[i].concatOrReplace == pathWithSlash)
				{
					//std::cout << "[DEBUG onResponse] Redirecting '" << requestedPath << "' to '" << pathWithSlash << "' with 301" << std::endl;
					std::ostringstream redirectResponse;
					redirectResponse << "HTTP/1.1 301 Moved Permanently\r\n"
						<< "Location: " << pathWithSlash << "\r\n"
						<< "Content-Length: 0\r\n"
						<< "Connection: close\r\n"
						<< "\r\n";
					response = redirectResponse.str();
					return;
				}
			}
		} 
		resolvePath(r, s, root, path, foundLoc);

		//ICI CE N'EST PAS UN CGI
		//response
		//std::cout << "[DEBUG onResponse] After resolvePath - root='" << root << "' path='" << path << "'" << std::endl;
		HttpResponse response1(r, r.getError());
		response1.setRoot(root);
		response1.setMatchedLocation(foundLoc);
		response1.setLocations(s.locations);
		//response1.setServerMethods(s.httpMethodsAllowed);
		response1.HttpResponseManager();
		response = response1.getFormatedResponse();
		//std::cout << Colors::CYAN << response << Colors::RESET << std::endl;
	}
}

int main(int argc, char *argv[])
{
	ConfigParser *cp = NULL;

	if (argc > 2)
	{
		std::cout << "Usage: [configuration file]" << std::endl;
		return (0);
	}

	struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

	//Config file parsing
	try
	{
		if (argc == 2)
			cp = new ConfigParser(argv[1]);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << "webserv: configuration file " 
			+ std::string(argv[1]) + " \e[0;33mtest\e[0m failed\n";
		return (1);
	}
	//Listener
	try
	{
		Webserv	webserv(cp);
		delete (cp);
		webserv.startListening(onResponse);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}
	return (0);
}