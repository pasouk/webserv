/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:43:17 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/25 16:24:49 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Node.hpp"

Node::Node(const std::string name) : name(name) {}
Node::~Node() {}

const std::vector<std::string>& Node::getArgs() const
{
	return (args);
}

void Node::setArgs(std::vector<std::string>& args)
{
	this->args = args;
}

const std::string& Node::getName() const
{
	return (name);
}