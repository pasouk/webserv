/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:14 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/24 13:43:49 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "NodeDirective.hpp"


NodeDirective::NodeDirective(Directives directive) : directive(directive) {}

NodeDirective::~NodeDirective() {}

const std::string& NodeDirective::getName() const
{
	return (directive.getName());
}