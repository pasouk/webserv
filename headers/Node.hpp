/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:42:30 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/30 07:28:15 by fbuyl            ###   ########.fr       */
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
	Node();
	Node(const std::string&, const std::string&);
	virtual ~Node();

	Node& operator=(const Node&);

	const std::string& getName() const;
	const std::string& getParent() const;
	const std::vector<std::string>& getArgs() const;
	void setArgs(std::vector<std::string>&);
	
protected:
	Directives m_block;
	std::string m_name;
	std::string m_parent;
	std::vector<std::string> m_args;
};

#endif