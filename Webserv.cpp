/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:29:06 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/31 14:50:57 by fbuyl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Node.hpp"

Webserv::Webserv(ConfigParser* parser)
{
	std::vector<Node*> serveurs;
	std::vector<Node*> listens;
	uint16_t port = 80;
	std::string host;

	if (parser == NULL)
		throw std::runtime_error("No configuration");

	serveurs = parser->getDirectives("server");
	for (std::vector<Node*>::const_iterator it = serveurs.begin(); it != serveurs.end(); ++it)
	{		
		listens = parser->getDirectives("listen", static_cast<NodeBlock*>(*it));
		if (listens.size() == 0)
		{
			std::cout << "NO LISTEN\n";
			listeners.push_back(new QueryListener());
		}
		else
		{
			std::cout << listens.size() << " LISTENERS\n";
			for (std::vector<Node*>::const_iterator it = listens.begin(); it != listens.end(); ++it)
			{
				static_cast<NodeDirective*>(*it)->getHostPort(port, host);
				std::cout << "host: " << host << ", port: " << port << std::endl;
				listeners.push_back(new QueryListener(port, host));
			}
		}
	}
	//std::cout << *config;
}

Webserv::~Webserv()
{
	for (std::vector<QueryListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
		delete (*it);
}

void Webserv::queriesListen() const
{

}