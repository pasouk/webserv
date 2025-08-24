/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directives.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 09:32:35 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/24 14:36:11 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTIVES_HPP
# define DIRECTIVES_HPP
# define MAX_ARGS 10

# include <iostream>
# include <vector>

class Directives
{
public:
	Directives(bool, int, std::string);
	const std::string& getName() const;
	bool isValid(const std::string&) const;

protected:
	bool		m_block;
	int			m_max_args;
	std::string	m_name;
	std::vector<std::string> m_memberships;
};

class Listen : public Directives { public: Listen(); };
class ServerName : public Directives { public: ServerName(); };
class Root : public Directives { public: Root(); };
class Index : public Directives { public: Index(); };
class Location : public Directives { public: Location(); };
class Server : public Directives { public: Server(); };
class Http : public Directives { public: Http(); };

#endif