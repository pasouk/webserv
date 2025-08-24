/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:42:30 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/24 13:43:01 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
# define NODE_HPP

# include <iostream>
# include <sstream>
# include <vector>

class Node
{
public:
	Node();
	virtual ~Node();

	virtual const std::string& getName() const = 0;
	const std::vector<std::string>& getArgs() const;
	void setArgs(std::stringstream&);
	
protected:
	std::vector<std::string> args;
};

#endif