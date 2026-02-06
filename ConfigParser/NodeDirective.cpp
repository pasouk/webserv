/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:14 by fabricebuyl       #+#    #+#             */
/*   Updated: 2026/02/06 10:38:08 by fabrice          ###   ########.fr       */
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

int NodeDirective::getClientSize(size_t& size) const
{
	std::stringstream ss;

	if (m_name != "client_body_buffer_size" 
		&& m_name != "client_header_buffer_size"
		&& m_name != "client_max_body_size")
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
	std::map<std::string, HttpMethod> met;

	if (m_name != "limit_except" || i >= getArgs().size())
		return (1);
	for (int method = GET; method != UNKNOWN; ++method)
		met[methods_map[method].name] = methods_map[method].value;
	m = met[getArgs()[i]];
	return (0);	
}

int NodeDirective::getGetBodyTempFile(std::string& file) const
{
	if (m_name != "client_body_temp_path" || !getArgs().size())
		return (1);
	file = getArgs()[0];
	if (mkdir(file.c_str(), 0755))
		return (-1);
	return (0);
}

