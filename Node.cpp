/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:43:17 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/30 14:48:38 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Node.hpp"

Node::Node(const std::string& name, Node* parent, int deep)
	: m_name(name), m_parent(parent), m_deep(deep) {}
Node::~Node() {}

const std::vector<std::string>& Node::getArgs() const
{
	return (m_args);
}

void Node::setArgs(std::vector<std::string>& args)
{
	this->m_args = args;
}

const std::string& Node::getName() const
{
	return (m_name);
}

Node* Node::getParent() const
{
	return (m_parent);
}

int Node::getDeep() const
{
	return (m_deep);
}