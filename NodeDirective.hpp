/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:16 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/24 12:15:09 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODEDIRECTIVE_HPP
# define NODEDIRECTIVE_HPP

# include "Node.hpp"
# include "Directives.hpp"

class NodeDirective : public Node
{
public:
	NodeDirective(Directives);
	~NodeDirective();
	const std::string& getName() const;

private:
	Directives directive;
};

#endif