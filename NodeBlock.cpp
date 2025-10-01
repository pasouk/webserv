/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeBlock.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:49:49 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/30 15:11:23 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "NodeBlock.hpp"
#include "NodeDirective.hpp"

NodeBlock::NodeBlock() : Node("ASP", NULL, 0) {}

NodeBlock::NodeBlock(const std::string& name, Node* parent, int deep)
	: Node(name, parent, deep) {}

NodeBlock::~NodeBlock()
{
	for (std::vector<NodeBlock*>::const_iterator it = m_blocks.begin(); it != m_blocks.end(); ++it)
		delete (*it);
	for (std::vector<NodeDirective*>::const_iterator it = m_directives.begin(); it != m_directives.end(); ++it)
		delete (*it);
}

const std::vector<NodeBlock*> NodeBlock::getBlocks() const
{
	return (m_blocks);
}

const std::vector<NodeDirective*> NodeBlock::getDirectives() const
{
	return (m_directives);
}

Node* NodeBlock::addChild(const Directives& directive, const std::string& name)
{
	Node* node;

	m_block = directive;
	if (directive.isBlock())
	{
		node = new (std::nothrow) NodeBlock(name, m_parent, m_deep + 1);
		return (m_blocks.push_back(static_cast<NodeBlock*>(node)), node);
	}
	node = new (std::nothrow) NodeDirective(name, m_parent, m_deep);
	return (m_directives.push_back(static_cast<NodeDirective*>(node)), node);
}