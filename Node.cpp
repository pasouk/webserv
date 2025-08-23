/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:43:17 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/23 11:45:04 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Node.hpp"

Node::Node(const std::string type) : type(type), args("") {}
Node::Node() : type(""), args("") {}
Node::~Node() {}

const std::string& Node::getType() const
{
	return (type);
}

const std::string& Node::getArgs() const
{
	return (args);
}

void Node::setArgs(std::string args)
{
	this->args = args;
}