/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeBlock.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:49:49 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/30 07:28:28 by fbuyl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "NodeBlock.hpp"
#include "NodeDirective.hpp"

NodeBlock::NodeBlock() : Node("ASP", "none") {}

NodeBlock::NodeBlock(const std::string& name, const std::string& parent) : Node(name, parent) {}

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
		node = new (std::nothrow) NodeBlock(name, m_parent);
		return (m_blocks.push_back(static_cast<NodeBlock*>(node)), node);
	}
	node = new (std::nothrow) NodeDirective(name, m_parent);
	return (m_directives.push_back(static_cast<NodeDirective*>(node)), node);
}