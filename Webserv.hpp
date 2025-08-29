/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 09:27:47 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/29 10:14:00 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "QueryListener.hpp"

class Webserv
{
public:
	Webserv(ConfigParser*);
	Webserv();
	~Webserv();
	Webserv(const Webserv&);

	Webserv& operator=(const Webserv&);

private:
	std::vector<QueryListener*> listeners;
};

#endif