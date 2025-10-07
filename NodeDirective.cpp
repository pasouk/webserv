/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:14 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/10/07 10:39:13 by fbuyl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "NodeDirective.hpp"
#include "NodeBlock.hpp"

//Here we assume that arguments (getArgs) parsing has passed !
NodeDirective::NodeDirective(const std::string& name, Node* parent, int deep)
	: Node(name, parent, deep) {}

NodeDirective::~NodeDirective() {}

int NodeDirective::getListenHostPort(uint16_t& port, std::string& host) const
{
	std::vector<std::string> args, splits;
	std::string split;
	std::stringstream ss;
	std::istringstream iss;

	if (m_name != "listen")
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

	if (m_name != "client_body_buffer_size" && m_name != "client_header_buffer_size")
		return (1);
	ss << getArgs()[0];
	ss >> size;
	return (0);
}

int NodeDirective::getClientsTimeout(size_t& size) const
{
	std::stringstream ss;

	if (m_name != "keepalive_timeout")
		return (1);
	ss << getArgs()[0];
	ss >> size;
	return (0);
}

int NodeDirective::getHttpMethod(size_t i, HttpMethod& m) const
{
	std::stringstream ss;
	int _int;

	if (m_name != "limit_except" || i >= getArgs().size())
		return (1);
	ss << getArgs()[i];
	ss >> _int;
	for (int method = GET; method != UNKNOWN; ++method)
	{
		if (_int == method)
		{
			m = static_cast<HttpMethod>(_int);
			break ;
		}
	}
	return (0);	
}

