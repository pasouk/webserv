/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 09:26:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/10/12 10:48:07 by fabricebuyl      ###   ########.fr       */
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
	std::string path;
	std::string root;
	size_t pos;
	location l;

	//MAXENCE: dans s.httpMethodsAllowed, tu as les methodes HTTP autorisée par les server.
	//2 posibilités:
	// - s.httpMethodsAllowed est vide (size() == 0): tout les methodes HTTP sont autorisées.
	// - si pas vide, si la methode demandée par le client n'est pas dans la liste -> 405 error
	
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

	//MAXENCE: ici j'implémente les directives alias/root dans la directive location (cfr nginx)
	//du coup j'ai ajouté un setter "setPath" a ta classe ParserHttpRequest qui modifie _path !
	//J'ai rajouter un server goldo.lu dans le fcihier default_location.conf qui applique ça.
	//Regarde y parceque cà ne marche pas, si tu n'as pas le temps d'y regarder,
	//je peux, mais tu connais mieux ton code.

	//alias/root location -> update path.
	root = s.root;
	if (root[0] == '/') //to get a relative path to the project.
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
					std::cout << "ROOT: " << path << std::endl;		
				}
				else if (s.locations[i].type == ALIAS)
				{
					path.replace(pos, s.locations[i].concatOrReplace.size(), s.locations[i].by);
					std::cout << "ALIAS: " << path << std::endl;
				}
				if (path[0] == '/') //to get a relative path to the project.
					path = path.substr(1, path.length() - 1);
				r.setPath(path);
				root = "";
			}
			else
				std::cout << "NOT FIND\n";
		}
	}

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