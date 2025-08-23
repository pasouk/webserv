/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:42:30 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/23 11:44:14 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
# define NODE_HPP

# include <iostream>

class Node
{
public:
	Node();
	Node(const std::string);
	virtual ~Node();

	virtual const std::string& getName() const = 0;
	const std::string& getType() const;
	const std::string& getArgs() const;
	void setArgs(std::string);
	
protected:
	std::string type;
	std::string args;
};

#endif