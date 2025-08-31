/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:14 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/31 11:21:22 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "NodeDirective.hpp"
#include "NodeBlock.hpp"

NodeDirective::NodeDirective(const std::string& name) : Node(name) {}

NodeDirective::~NodeDirective() {}

void NodeDirective::getHostPort(const uint16_t& port, const std::string& host) const
{
	(void)port;
	(void)host;
	size_t found;

	std::vector<std::string>::const_iterator it;
	std::vector<std::string> args;
	
	args = getArgs();
	for (it = args.begin(); it != args.end(); ++it)
	{
		found=(*it).find(':');
  		if (found != std::string::npos)
		{
			
			return ;
		}
	}
}