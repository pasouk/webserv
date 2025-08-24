/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directives.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 09:32:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/24 14:48:06 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Directives.hpp"

Directives::Directives(bool block, int max, std::string name) : m_block(block), m_max_args(max), m_name(name) {}

const std::string& Directives::getName() const
{
	return (m_name);
}

bool Directives::isValid(const std::string& parent) const
{
	std::vector<std::string>::const_iterator it;

	for (it = m_memberships.begin(); it != m_memberships.end(); ++it)
		if (parent == *it)
			return (true);
	return (false);
}

Listen::Listen() : Directives(false, MAX_ARGS, "listen")
{
	m_memberships.push_back("server");
}

ServerName::ServerName() : Directives(false, MAX_ARGS, "server_name")
{
	m_memberships.push_back("server");
}

Root::Root() : Directives(false, 1, "root")
{
	m_memberships.push_back("http");
	m_memberships.push_back("server");
	m_memberships.push_back("location");
}

Index::Index() : Directives(false, MAX_ARGS, "index")
{
	m_memberships.push_back("http");
	m_memberships.push_back("server");
	m_memberships.push_back("location");
}

Location::Location() : Directives(true, 1, "location")
{
	// If childs, parent must be a regular expression 
	m_memberships.push_back("location");
	m_memberships.push_back("server");
}

Server::Server() : Directives(true, 0, "server")
{
	m_memberships.push_back("http"); //HTTP/HTTPS
	m_memberships.push_back("stream"); //TCP/UDP
}

Http::Http() : Directives(true, 0, "http")
{
	m_memberships.push_back("ASP"); //principal context
}