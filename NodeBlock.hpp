/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeBlock.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:49:51 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/24 13:45:44 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODEBLOCK_HPP
# define NODEBLOCK_HPP

# include "Node.hpp"
# include "NodeDirective.hpp"
# include "Directives.hpp"

class NodeBlock : public Node
{
public:
	NodeBlock();
	~NodeBlock();
	void addBlock(const Directives&);
	const std::string& getName() const;
	const std::vector<NodeBlock*> getChilds() const;
	const std::vector<NodeDirective*> getDirectives() const;
	NodeBlock* addChild();
	NodeDirective* addDirective(const Directives&);
	
private:
	Directives block;
	std::vector<NodeDirective*> directives;
	std::vector<NodeBlock*> nodes;
};

#endif