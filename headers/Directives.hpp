/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directives.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 09:32:35 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/10/07 09:20:39 by fbuyl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTIVES_HPP
# define DIRECTIVES_HPP
# define MAX_ARGS 10

# include <iostream>
# include <vector>
# include <regex.h> //not boost or external library, it's a standard POSIX library
# include "ParserHttpRequest.hpp"

class Directives
{
public:
	Directives();
	virtual ~Directives();
	Directives(bool, int, int, std::string);
	
	const std::string& getName() const;
	size_t getMaxArgs() const;
	size_t getMinArgs() const;
	bool isMembership(const std::string&) const;
	bool isBlock() const;
	virtual bool areArgsValid(const std::vector<std::string>&, std::string&) const;

protected:
	bool		m_block;
	size_t		m_min_args;
	size_t		m_max_args;
	std::string	m_name;
	std::vector<std::string> m_memberships;
};

class Listen : public Directives
{
public: 
	Listen(); 
	bool areArgsValid(const std::vector<std::string>&, std::string&) const;
};
class ServerName : public Directives
{
public:
	ServerName();
	bool areArgsValid(const std::vector<std::string>&, std::string&) const;
};
class Alias : public Directives
{
public:
	Alias();
	bool areArgsValid(const std::vector<std::string>&, std::string&) const;
};
class Root : public Directives
{
public:
	Root();
	bool areArgsValid(const std::vector<std::string>&, std::string&) const;
};
class Index : public Directives { public: Index(); };
class Location : public Directives
{
public:
	Location();
	bool areArgsValid(const std::vector<std::string>&, std::string&) const;
};
class Server : public Directives { public: Server(); };
class Http : public Directives { public: Http(); };
class ClientBodyBufferSize : public Directives { public: ClientBodyBufferSize(); };
class ClientHeaderBufferSize : public Directives { public: ClientHeaderBufferSize(); };
class KeepaliveTimeout : public Directives { public: KeepaliveTimeout(); };
class Events : public Directives { public: Events(); }; //not use, only for nginx tests
class WorkerConnections : public Directives //not use, only for nginx tests
{
public: 
	WorkerConnections(); 
	bool areArgsValid(const std::vector<std::string>&, std::string&) const;
};
class LimitExcept : public Directives { public: LimitExcept(); };
class Deny : public Directives //only "all" argument is valid for now, not use, only for nginx tests
{ 
public:
	Deny();
	bool areArgsValid(const std::vector<std::string>&, std::string&) const;
};

#endif