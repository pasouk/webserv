/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeBlock.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:49:49 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/28 09:12:30 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "NodeBlock.hpp"
#include "NodeDirective.hpp"

NodeBlock::NodeBlock() : Node("ASP") {}

NodeBlock::NodeBlock(const std::string& name) : Node(name) {}

NodeBlock::~NodeBlock()
{
	for (std::vector<NodeBlock*>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
		delete (*it);
	for (std::vector<NodeDirective*>::const_iterator it = directives.begin(); it != directives.end(); ++it)
		delete (*it);
}

const std::vector<NodeBlock*> NodeBlock::getBlocks() const
{
	return (blocks);
}

const std::vector<NodeDirective*> NodeBlock::getDirectives() const
{
	return (directives);
}

Node* NodeBlock::addChild(const Directives& directive, const std::string& name)
{
	Node* node;

	block = directive;
	if (directive.isBlock())
	{
		node = new (std::nothrow) NodeBlock(name);
		return (blocks.push_back(static_cast<NodeBlock*>(node)), node);
	}
	node = new (std::nothrow) NodeDirective(name);
	return (directives.push_back(static_cast<NodeDirective*>(node)), node);
}