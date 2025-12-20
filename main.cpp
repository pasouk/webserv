/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 09:26:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/11/22 13:48:03 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "ParserHttp.hpp"
#include "HTTP_response/HttpResponse.hpp"


#include <string> //remove it

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
    std::cout << "[DEBUG resolvePath] Initial - path='" << mappedPath << "' root='" << root << "'" << std::endl;

    if (s.locations.empty())
    {
        std::cout << "[DEBUG resolvePath] No locations, returning with root='" << root << "'" << std::endl;
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
        std::cout << "[DEBUG resolvePath] No matching location, using default root='" << root << "'" << std::endl;
        foundLocation = s_location();  // Initialize with empty location
        return;
    }

    const s_location &loc = s.locations[bestIndex];
    foundLocation = loc;  // Store the found location
    std::cout << "[DEBUG resolvePath] Matched location " << bestIndex << " with prefix='" << loc.concatOrReplace << "'" << std::endl;

    // Remainder after removing prefix
    size_t prefix_len = loc.concatOrReplace.size();
    std::string remainder;

    if (mappedPath.size() > prefix_len)
        remainder = mappedPath.substr(prefix_len);

    // --------------------------
    // 2) TYPE ROOT
    // --------------------------
    if (loc.type == ROOT)
    {
        root = loc.by;
        std::cout << "[DEBUG resolvePath] ROOT location - new root='" << root << "'" << std::endl;

        std::string newReqPath = remainder;

        if (newReqPath.empty())
            newReqPath = "/";
        else if (newReqPath[0] != '/')
            newReqPath = "/" + newReqPath;

        std::cout << "[DEBUG resolvePath] ROOT location - new path='" << newReqPath << "'" << std::endl;
        r.setPath(newReqPath);
        mappedPath = newReqPath;
    }

    // --------------------------
    // 3) TYPE ALIAS
    // --------------------------
    else if (loc.type == ALIAS)
    {
        std::string newReqPath = loc.by;
        std::cout << "[DEBUG resolvePath] ALIAS location - new path='" << newReqPath << "' root='" << root << "'" << std::endl;

        r.setPath(newReqPath);
        mappedPath = newReqPath;
    }
    std::cout << "[DEBUG resolvePath] Final - path='" << mappedPath << "' root='" << root << "'" << std::endl;
}

void onResponse(std::string& response, CGI* cgi, ParserHttpRequest& r, s_server& s)
{	
	std::ostringstream oss;
	std::stringstream ss;
	std::string path;
	std::string root;
	s_location foundLoc;

	(void)cgi;
	//MAXENCE:
	//si cgi different de NULL:
	//cela veux dire qu'un CGI a été crée mais PAS ENCORE EXCUTE.
	//il a été crée parceque la requete A MATCH UNE LOCATION CONTENANT UNE DIRECTIVE "cgi_pass"
	if (cgi)
	{
		if (cgi->wasExecuted() == false)
		{
			std::cerr << "WE HAVE A CGI\n";
			//TODO
			//ici tu peux faire ce que tu veux avant l'execution du CGI
			if (cgi->runCGI())
			{
				oss << "client fd:" << cgi->getFd() << ", cgi failed to run";
				logErrMessage(oss);

				//TODO:
				//il y a eu une erreur, CGI détruit.
			}
		}
		else
		{
			//TODO
			//ici l'execution c'est bien déroulée et tu as la reponse du CGI avec cgi->getResponse()
			//je renvoi ca pour mes tests, libre a toi de modifier.
			ss << cgi->getResponse().length();
			std::string responseBuild =
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: " + ss.str() + "\r\n"
				"Connection: close\r\n"
				"\r\n"
				+ cgi->getResponse();
			response = responseBuild;
		}
	}
	else
	{
		resolvePath(r, s, root, path, foundLoc);

		//ICI CE N'EST PAS UN CGI
		//response
		std::cout << "[DEBUG onResponse] After resolvePath - root='" << root << "' path='" << path << "'" << std::endl;
		HttpResponse response1(r, r.getError());
		response1.setRoot(root);
		response1.setMatchedLocation(foundLoc);
		response1.setLocations(s.locations);
		//response1.setServerMethods(s.httpMethodsAllowed);
		response1.HttpResponseManager();
		response = response1.getFormatedResponse();
		std::cout << Colors::CYAN << response << Colors::RESET << std::endl;
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
		//webserv.printServers();
		delete (cp);
		webserv.startListening(onResponse);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}
	//delete (cp);
	return (0);
}