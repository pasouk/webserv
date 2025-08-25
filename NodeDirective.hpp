/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:16 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/23 13:57:47 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODEDIRECTIVE_HPP
# define NODEDIRECTIVE_HPP

# include "Node.hpp"

class NodeDirective : public Node
{
public:
	NodeDirective(std::string);
	~NodeDirective();
	const std::string& getName() const;

private:
	std::string directive;
};

#endif