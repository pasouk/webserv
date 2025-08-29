/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:29:06 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/29 10:21:13 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Node.hpp"

Webserv::Webserv(ConfigParser* parser)
{
	std::vector<Node*> serveurs;
	std::vector<Node*> listens;

	if (parser == NULL)
		throw std::runtime_error("No configuration");

	serveurs = parser->getDirectives("server");
	for (std::vector<Node*>::const_iterator it = serveurs.begin(); it != serveurs.end(); ++it)
	{		
		listens = parser->getDirectives("listen", static_cast<NodeBlock*>(*it));
		for (std::vector<Node*>::const_iterator it = listens.begin(); it != listens.end(); ++it)
			listeners.push_back(new QueryListener());
	}
	//std::cout << *config;
}

Webserv::~Webserv()
{
	for (std::vector<QueryListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
		delete (*it);
}