/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:42:30 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/25 16:24:30 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
# define NODE_HPP

# include <iostream>
# include <sstream>
# include <vector>
# include "Directives.hpp"

class Node
{
public:
	Node(const std::string);
	virtual ~Node();

	virtual const std::string& getName() const;
	const std::vector<std::string>& getArgs() const;
	void setArgs(std::vector<std::string>&);
	
protected:
	Directives block;
	std::string name;
	std::vector<std::string> args;
};

#endif