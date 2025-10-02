/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 09:26:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/30 10:14:33 by fbuyl            ###   ########.fr       */
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

void onQuery(query& q, std::vector<server>& s, Webserv* ser)
{
	(void)s;
	(void)q;
	(void)ser;

	//std::string root = s[1].root; //Normalement ceci devrait fonctionner mais j'avais mis la ligne suivante pour adapter a mon pc dans modifier ton fichier config 
	//std::string root = "/home/pasouk/webserv";
	std::string root;
	for (std::vector<server>::const_iterator server = s.begin(); server != s.end(); ++server)
	{
		std::vector<uint16_t>::const_iterator port;
		for (port = (*server).ports.begin(); port != (*server).ports.end(); ++port)
			if (*port == q.port)
			{
				root = (*server).root;
				/*for (std::map<std::string, std::string>::const_iterator loc = (*server).locations.begin()
					; loc != (*server).locations.end(); ++loc)
					std::cout << (*loc).first << ": " << (*loc).second << std::endl;*/
				break;
			}
		if (port != (*server).ports.end())
			break;
	}
	//MAXENCE: par defaut nginx doit contenir un chemin absolu et commencer par '/', si ce n'est pas le cas,
	//mon parser génère une erreur, donc je le supprime après parceque ta solution fonctionne sans.
	if (root[0] == '/')
		root = root.substr(1, root.length() - 1);

	//parsing
	ParserHttpRequest request1(q.httpRequest);

    int ret = request1.parseRequest();   // Attention : utiliser soit parserequest + printparsingdata soit debugparsingdata tout seul
	//std::cout << Colors::RED << "-----------Parsed data----------\n" << Colors::RESET;
	//int ret = request1.debugParsingRequest();
	//request1.printParsedData();

	//response
    HttpResponse response1(request1, ret);
    response1.setRoot(root);
    response1.HttpResponseManager();
	//std::cout << Colors::RED << "\n\n\n ----------Response ----------\n" << Colors::RESET;
    //response1.printElements();

	//std::cout << "\n\n------------------------------------------------\n\n";
	// FABRICE : quand ce sera pret : 
	q.formatedResponse = response1.getFormatedResponse();

	//ser->printQuery(q);
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
		webserv.startListening(onQuery);
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