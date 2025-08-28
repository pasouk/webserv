/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:29:06 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/28 15:32:11 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Node.hpp"

Webserv::Webserv(ConfigParser* config) : m_parser(config)
{
	std::vector<Node*> serveurs;
	std::vector<Node*> listens;

	if (m_parser == NULL)
		throw std::runtime_error("No configuration");

	serveurs = m_parser->getDirectives("server");
	for (std::vector<Node*>::const_iterator it = serveurs.begin(); it != serveurs.end(); ++it)
	{		
		std::cout << (*it)->getName() << std::endl;
		listens = m_parser->getDirectives("listen", static_cast<NodeBlock*>(*it));
		for (std::vector<Node*>::const_iterator it = listens.begin(); it != listens.end(); ++it)
			std::cout << "  ." << (*it)->getName() << " " << (*it)->getArgs()[0] << std::endl;
	}
	//std::cout << *config;
}

Webserv::~Webserv() {}