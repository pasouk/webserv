/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directives.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 09:32:33 by fabricebuyl       #+#    #+#             */
/*   Updated: 2026/02/12 11:16:58 by fabrice          ###   ########.fr       */
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

bool Directives::areArgsValid(const std::vector<std::string>& args, std::string&) const
{
	(void)args;
	return (true);
}

Listen::Listen() : Directives(false, 0, 2, "listen")
{
	m_memberships.push_back("server");
}

bool Listen::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t regex;
	std::string pattern(
		//url + port or port
		"(^((25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})(\\.(25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})){3}"
		"(:(6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[0-9]{1,4}))?)$)"
		//only port
		"|(^(6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[0-9]{1,4})$)"
	);
	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (err = (*it), regfree(&regex), false);
	}
	return (regfree(&regex), true);
}

ServerName::ServerName() : Directives(false, 0, MAX_ARGS, "server_name")
{
	m_memberships.push_back("server");
}

Alias::Alias() : Directives(false, 1, 1, "alias")
{
	m_memberships.push_back("location");
}

bool Alias::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t regex;
	std::string pattern("^(?:/|\\.\\/|\\.\\./)?(?:[A-Za-z0-9._*?-]+/)*$");
	
	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (err = (*it), regfree(&regex), false);
	}
	return (regfree(&regex), true);
}

Root::Root() : Directives(false, 1, 1, "root")
{
	m_memberships.push_back("http");
	m_memberships.push_back("server");
	m_memberships.push_back("location");
}

bool Root::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t regex;
	std::string pattern("^(?:/|\\.\\/|\\.\\./)?(?:[A-Za-z0-9._*?-]+/)*$");
	
	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (err = (*it), regfree(&regex), false);
	}
	return (regfree(&regex), true);
}

Index::Index() : Directives(false, 1, MAX_ARGS, "index")
{
	m_memberships.push_back("http");
	m_memberships.push_back("server");
	m_memberships.push_back("location");
}

Location::Location() : Directives(true, 1, 1, "location")
{
	m_memberships.push_back("server");
}

bool Location::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t regex;
	std::string pattern("^(/([A-Za-z0-9._*?-]+/)*([A-Za-z0-9._*?-]+)?|[A-Za-z0-9._*?-]+)$");

	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (err = (*it), regfree(&regex), false);
	}
	return (regfree(&regex), true);
}

Server::Server() : Directives(true, 0, 0, "server")
{
	m_memberships.push_back("http"); //HTTP/HTTPS
}

Http::Http() : Directives(true, 0, 0, "http")
{
	m_memberships.push_back("ASP"); //principal context
}

ClientBodyBufferSize::ClientBodyBufferSize() : Directives(false, 1, 1, "client_body_buffer_size")
{
	m_memberships.push_back("http");
}

bool ClientBodyBufferSize::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t regex;
	std::string pattern("^[0-9]{1,10}$");
	
	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (err = (*it), regfree(&regex), false);
	}
	return (regfree(&regex), true);
}

ClientHeaderBufferSize::ClientHeaderBufferSize() : Directives(false, 1, 1, "client_header_buffer_size")
{
	m_memberships.push_back("http");
}

bool ClientHeaderBufferSize::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t regex;
	std::string pattern("^[0-9]{1,10}$");
	
	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (err = (*it), regfree(&regex), false);
	}
	return (regfree(&regex), true);
}

ClientBodyTempPath::ClientBodyTempPath() : Directives(false, 1, 1, "client_body_temp_path")
{
	m_memberships.push_back("http");
}

bool ClientBodyTempPath::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t regex;
	std::string pattern("^(?:/|\\.\\/|\\.\\./)?(?:[A-Za-z0-9._*?-]+/)*$");
	
	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (err = (*it), regfree(&regex), false);
	}
	return (regfree(&regex), true);
}

KeepaliveTimeout::KeepaliveTimeout() : Directives(false, 1, 1, "keepalive_timeout")
{
	m_memberships.push_back("http");
}

bool KeepaliveTimeout::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t regex;
	std::string pattern("^[0-9]{1,5}$");
	
	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (err = (*it), regfree(&regex), false);
	}
	return (regfree(&regex), true);
}

ClientMaxBodySize::ClientMaxBodySize() : Directives(false, 1, 1, "client_max_body_size")
{
	m_memberships.push_back("http");
	m_memberships.push_back("server");
	m_memberships.push_back("location");
}

bool ClientMaxBodySize::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t regex;
	std::string pattern("^[0-9]{1,10}$");
	
	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (err = (*it), regfree(&regex), false);
	}
	return (regfree(&regex), true);
}

Events::Events() : Directives(true, 0, 0, "events")
{
	m_memberships.push_back("ASP");
}

LimitExcept::LimitExcept() : Directives(true, 1, 10, "limit_except")
{
	m_memberships.push_back("location");
}

Deny::Deny() : Directives(false, 1, 1, "deny")
{
	m_memberships.push_back("limit_except");
}

bool Deny::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	if (args[0] == "all")
		return (true);
	return (err = "only \"all\" argument is allowed for now.", false);
}

CgiPass::CgiPass() : Directives(false, 0, 1, "cgi_pass")
{
	m_memberships.push_back("location");
}

bool CgiPass::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t regex;
	std::string pattern("^(/|\\.\\/|\\.\\./)?([A-Za-z0-9._*?-]+(/)?)*$");
	
	if(regcomp(&regex, pattern.c_str(), REG_EXTENDED) == 0)
	{
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
			if(regexec(&regex, (*it).c_str(), 0, NULL, 0) == REG_NOMATCH)
				return (err = (*it), regfree(&regex), false);
	}
	return (regfree(&regex), true);
}

// [CHANGED] New directive: autoindex on/off — enables directory listing when no index file is found
Autoindex::Autoindex() : Directives(false, 1, 1, "autoindex")
{
	m_memberships.push_back("location");
}

bool Autoindex::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	if (args[0] == "on" || args[0] == "off")
		return (true);
	return (err = "autoindex: invalid argument '" + args[0] + "' (must be on or off)", false);
}

// [CHANGED] New directive: return <code> [url] — emits a redirect response from the config
ReturnDirective::ReturnDirective() : Directives(false, 1, 2, "return")
{
	m_memberships.push_back("location");
}

bool ReturnDirective::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	for (size_t i = 0; i < args[0].size(); ++i)
		if (!std::isdigit(static_cast<unsigned char>(args[0][i])))
			return (err = "return: first argument must be a status code", false);
	return (true);
}

// [CHANGED] New directive: error_page <code>... <path> — maps HTTP error codes to custom HTML pages
ErrorPage::ErrorPage() : Directives(false, 2, MAX_ARGS, "error_page")
{
	m_memberships.push_back("http");
	m_memberships.push_back("server");
	m_memberships.push_back("location");
}

bool ErrorPage::areArgsValid(const std::vector<std::string>& args, std::string& err) const
{
	regex_t pathRegex;
	bool regexCompiled;
	std::string pathPattern("^(?:/|\\./|\\../)?(?:[A-Za-z0-9._*?-]+/)*[A-Za-z0-9._*?-]*$");

	regexCompiled = false;

	for (size_t i = 0; i + 1 < args.size(); ++i)
	{
		if (args[i].size() != 3)
			return (err = args[i], false);
		for (size_t j = 0; j < args[i].size(); ++j)
			if (!std::isdigit(static_cast<unsigned char>(args[i][j])))
				return (err = args[i], false);
	}

	if(regcomp(&pathRegex, pathPattern.c_str(), REG_EXTENDED) == 0)
	{
		regexCompiled = true;
		if (regexec(&pathRegex, args[args.size() - 1].c_str(), 0, NULL, 0) == REG_NOMATCH)
			return (err = args[args.size() - 1], regfree(&pathRegex), false);
	}
	if (regexCompiled)
		regfree(&pathRegex);
	return (true);
}
