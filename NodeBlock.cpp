/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeBlock.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:49:49 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/24 13:45:44 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "NodeBlock.hpp"

NodeBlock::NodeBlock() : block(true, 0, "ASP") {}

NodeBlock::~NodeBlock()
{
	for (std::vector<NodeBlock*>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		delete (*it);
	for (std::vector<NodeDirective*>::const_iterator it = directives.begin(); it != directives.end(); ++it)
		delete (*it);
}

void NodeBlock::addBlock(const Directives& block)
{
	this->block = block;
}

const std::string& NodeBlock::getName() const
{
	return (block.getName());
}

const std::vector<NodeBlock*> NodeBlock::getChilds() const
{
	return (nodes);
}

const std::vector<NodeDirective*> NodeBlock::getDirectives() const
{
	return (directives);
}

NodeBlock* NodeBlock::addChild()
{
	NodeBlock* newBlock;

	newBlock = new (std::nothrow) NodeBlock();
	return (nodes.push_back(newBlock), newBlock);
}

NodeDirective* NodeBlock::addDirective(const Directives& directive)
{
	NodeDirective* newDirective;

	newDirective = new (std::nothrow) NodeDirective(directive);
	return (directives.push_back(newDirective), newDirective);
}