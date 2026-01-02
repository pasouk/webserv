/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:42:30 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/30 14:48:35 by fabricebuyl      ###   ########.fr       */
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
	Node(const std::string&, Node*, int);
	virtual ~Node();

	Node& operator=(const Node&);

	Node* getParent() const;
	int getDeep() const;
	const std::string& getName() const;
	const std::vector<std::string>& getArgs() const;
	void setArgs(std::vector<std::string>&);
	
protected:
	std::string m_name;
	Node* m_parent;
	int	m_deep;
	Directives m_block;
	std::vector<std::string> m_args;
};

#endif