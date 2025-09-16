/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 09:26:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/16 14:11:39 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "ParserHttp.hpp"

bool g_listening = true;

void handle_sigint(int sig)
{
	(void)sig;
	g_listening = false;
}

void onContentLength(query& q, Webserv* server)
{
	(void)q;
	(void)server;

    /*ParserHttpRequest request1(q.httpRequest);
   	int ret = request1.parseRequest();
	if(ret)
    {
        std::cout << Colors::RED << "Parsing exit code : " << ret << Colors::RESET << std::endl;
		q.bodySize = 25; //request1.getBodySize();
	}
    else
	{
        std::cout << Colors::GREEN << "Parsing exit code : " << ret << Colors::RESET << std::endl;
		q.bodySize = 25; //request1.getBodySize();
	}*/
	q.bodySize = 25;
}

void onQueries(std::vector<query>& q, std::vector<server>& s, Webserv* server)
{
	(void)s;
	(void)q;
	(void)server;

	if (q.size())
		do
		{
			server->printQuery(q.back());
			std::cout << "------------------\n";
			q.pop_back();
		} while (q.size());
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
		webserv.startListening(onContentLength, onQueries);
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