/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 09:26:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/10/08 14:40:09 by fabricebuyl      ###   ########.fr       */
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
	std::string root = s.root;
	location l;

	//aurorisd http methods
	if (s.httpMethodsAllowed.size() == 0)
		std::cout << "ALL HTTP METHODS ARE ALLOWED.\n";
	else
	{
		std::cout << "ONLY ALLOWED METHOD(S) ON SERVER: ";
		for (size_t i = 0; i < s.httpMethodsAllowed.size(); ++i)
			std::cout << methods_map[s.httpMethodsAllowed[i]].name << " ";
		std::cout << std::endl;
	}
	//alias/root location
	if (s.locations.size())
	{
		for (size_t i = 0; i < s.locations.size(); ++i)
			if (s.locations[i].type == ROOT)
				std::cout << "IN PATH, CONCAT " << s.locations[i].concatOrReplace << " BY " << s.locations[i].by << std::endl;
			else if (s.locations[i].type == ALIAS)
				std::cout << "IN PATH, REPLACE " << s.locations[i].concatOrReplace << " BY " << s.locations[i].by << std::endl;
	}


	if (root[0] == '/')
		root = root.substr(1, root.length() - 1);

	
	//response
    HttpResponse response1(r, r.getError());
    response1.setRoot(root);
    response1.HttpResponseManager();

	response = response1.getFormatedResponse();
}

int main(int argc, char *argv[])
{
	ConfigParser *cp;

	if (argc != 2)
	{
		std::cout << "Usage: <configuration file>" << std::endl;
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
		webserv.printServers();
		webserv.startListening(onResponse);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		delete (cp);
		return (1);
	}
	delete (cp);
	return (0);
}