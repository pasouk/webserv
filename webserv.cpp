/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 09:26:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/18 15:26:03 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "QueryListener.hpp"

#include "ConfigParser.hpp" //remove when tested

bool g_listening = true;

void handle_sigint(int sig)
{
	(void)sig;
	g_listening = false;
}

int main(int argc, char *argv[])
{
	//(void)argv;
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
	
	try
	{
		ConfigParser cp(argv[1]);
		//std::map<std::string, std::string> dirs = cp.getDirective();
		//for(std::map<std::string, std::string>::iterator it = dirs.begin(); it != dirs.end(); ++it)
		//	std::cout << it->first << ": " << it->second << std::endl;

		cp.getFormat();

		//QueryListener listener;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}

	return (0);
}