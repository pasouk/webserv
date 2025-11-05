/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 09:26:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/11/05 07:14:41 by fabrice          ###   ########.fr       */
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

void onResponse(std::string& response, ParserHttpRequest& r, server& s)
{	
	(void)response;
	std::string path;
	std::string root;
	size_t pos;

	//alias/root location -> update path.
	root = s.root;
	if (RELATIVE) //to get a relative path to the project.
		if (root[0] == '/')
			root = root.substr(1, root.length() - 1);
	if (s.locations.size())
	{
		path = r.getPath();
		for (size_t i = 0; i < s.locations.size(); ++i)
		{
			pos = path.find(s.locations[i].concatOrReplace);
			if (pos != std::string::npos)
			{
				if (s.locations[i].type == ROOT)
				{
					path.replace(pos, s.locations[i].concatOrReplace.size()
						, s.locations[i].by + s.locations[i].concatOrReplace);
					//std::cout << "LOCATION " << i << ": ROOT: " << path << std::endl;
				}
				else if (s.locations[i].type == ALIAS)
				{
					path.replace(pos, s.locations[i].concatOrReplace.size(), s.locations[i].by);
					//std::cout << "LOCATION " << i << ": ALIAS: " << path << std::endl;
				}
				if (RELATIVE) //to get a relative path to the project.
					if (path[0] == '/')
						path = path.substr(1, path.length() - 1);
				r.setPath(path);
				root = "";
			}
			/*std::cout << "LOCATION " << i << ": MAX BODY SIZE: " << s.locations[i].max_body_size << std::endl;
			std::cout << "LOCATION " << i << ": ALLOWED METHOD(S): " << std::endl;
			for (size_t j = 0; j < s.locations[i].httpMethodsAllowed.size(); ++j)
				std::cout << methods_map[s.locations[i].httpMethodsAllowed[j]].name << " ";
			std::cout << std::endl;*/
		}
	}
	//response
    HttpResponse response1(r, r.getError());
    response1.setRoot(root);
	//response1.setServerMethods(s.httpMethodsAllowed);
    response1.HttpResponseManager();
	response = response1.getFormatedResponse();
	//std::cout << response << std::endl;
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