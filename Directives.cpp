/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directives.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 09:32:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/29 15:08:49 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Directives.hpp"

Directives::Directives() : m_block(false), m_min_args(0), m_max_args(MAX_ARGS), m_name("") {}
Directives::Directives(bool block, int min, int max, std::string name) : m_block(block), m_min_args(min), m_max_args(max), m_name(name) {}
Directives::~Directives() {}

const std::string& Directives::getName() const
{
	return (m_name);
}

bool Directives::isMembership(const std::string& parent) const
{
	std::vector<std::string>::const_iterator it;

	for (it = m_memberships.begin(); it != m_memberships.end(); ++it)
		if (parent == *it)
			return (true);
	return (false);
}

bool Directives::isBlock() const
{
	return (m_block);
}

size_t Directives::getMaxArgs() const
{
	return (m_max_args);
}

size_t Directives::getMinArgs() const
{
	return (m_min_args);
}

bool Directives::areArgsValid(const std::vector<std::string>& args) const
{
	(void)args;
	return (true);
}

Listen::Listen() : Directives(false, 0, MAX_ARGS, "listen")
{
	m_memberships.push_back("server");
}

bool Listen::areArgsValid(const std::vector<std::string>& args) const
{
	regex_t regex;
	std::string pattern("^((25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})(\\.(25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})){3}(:[0-9]+)?)$|^([0-9]+)$");
	
	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (regfree(&regex), false);
	}
	else
		std::cout << "LOULOU\n";
	return (regfree(&regex), true);
}

ServerName::ServerName() : Directives(false, 0, MAX_ARGS, "server_name")
{
	m_memberships.push_back("server");
}

Root::Root() : Directives(false, 1, 1, "root")
{
	m_memberships.push_back("http");
	m_memberships.push_back("server");
	m_memberships.push_back("location");
}

Index::Index() : Directives(false, 1, MAX_ARGS, "index")
{
	m_memberships.push_back("http");
	m_memberships.push_back("server");
	m_memberships.push_back("location");
}

Location::Location() : Directives(true, 1, 1, "location")
{
	// If childs, parent must be a regular expression 
	m_memberships.push_back("location");
	m_memberships.push_back("server");
}

Server::Server() : Directives(true, 0, 0, "server")
{
	m_memberships.push_back("http"); //HTTP/HTTPS
	//m_memberships.push_back("stream"); //TCP/UDP
}

Http::Http() : Directives(true, 0, 0, "http")
{
	m_memberships.push_back("ASP"); //principal context
}