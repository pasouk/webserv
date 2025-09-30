/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:43:17 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/30 07:28:15 by fbuyl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Node.hpp"

Node::Node(const std::string& name, const std::string& parent) : m_name(name), m_parent(parent) {}
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