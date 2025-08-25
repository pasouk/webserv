/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:42:30 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/23 15:12:29 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
# define NODE_HPP

# include <iostream>
# include <sstream>

class Node
{
public:
	Node();
	Node(const std::string);
	virtual ~Node();

	virtual const std::string& getName() const = 0;
	const std::string& getType() const;
	const std::vector<std::string>& getArgs() const;
	void setArgs(std::stringstream&);
	
protected:
	std::string type;
	std::vector<std::string> args;
};

#endif