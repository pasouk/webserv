/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:14 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/26 09:19:32 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "NodeDirective.hpp"
#include "NodeBlock.hpp"

NodeDirective::NodeDirective(const std::string& name) : Node(name) {}

NodeDirective::~NodeDirective() {}

//Here we assume that arguments (getArgs) parsing has passed !
int NodeDirective::getListenHostPort(uint16_t& port, std::string& host) const
{
	std::vector<std::string> args, splits;
	std::string split;
	std::stringstream ss;
	std::istringstream iss;

	if (name != "listen")
		return (1);
	args = getArgs();
	for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
	{
		iss.clear();
		iss.str(*it);
		while (std::getline(iss, split, ':'))
			splits.push_back(split);
		if (splits.size() > 1)
		{
			if  (splits[0].find('.') != std::string::npos)
			{
				host = splits[0];
				ss << splits[1];
				ss >> port;
			}
			else
			{
				host = splits[1];
				ss << splits[0];
				ss >> port;
			}
			return (0);
		}
		else
		{
			if ((*it).find('.') != std::string::npos)
				host = *it;
			else
			{
				ss << *it;
				ss >> port;
			}
		}
	}
	return (0);
}

int NodeDirective::getClientBufferSize(size_t& size) const
{
	std::stringstream ss;

	if (name != "client_body_buffer_size" && name != "client_header_buffer_size")
		return (1);
	ss << getArgs()[0];
	ss >> size;
	return (0);
}

int NodeDirective::getClientsTimeout(size_t& size) const
{
	std::stringstream ss;

	if (name != "keepalive_timeout")
		return (1);
	ss << getArgs()[0];
	ss >> size;
	return (0);
}

