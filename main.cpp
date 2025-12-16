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

void onResponse(std::string& response, CGI* cgi, ParserHttpRequest& r, s_server& s)
{	
	std::ostringstream oss;
	std::stringstream ss;

	//MAXENCE:
	//si cgi different de NULL:
	//cela veux dire qu'un CGI a été crée mais PAS ENCORE EXCUTE.
	//il a été crée parceque la requete A MATCH UNE LOCATION CONTENANT UNE DIRECTIVE "cgi_pass"
	if (cgi)
	{
		if (cgi->wasExecuted() == false)
		{
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
		//ICI CE N'EST PAS UN CGI
		//response
		HttpResponse response1(r, r.getError());
		response1.setRoot(s.root);
		//response1.setServerMethods(s.httpMethodsAllowed);
		response1.HttpResponseManager();
		response = response1.getFormatedResponse();
		//std::cout << response << std::endl;
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