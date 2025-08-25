/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeBlock.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:49:51 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/25 16:12:13 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODEBLOCK_HPP
# define NODEBLOCK_HPP

# include "Node.hpp"
# include "NodeDirective.hpp"
//# include "Directives.hpp"

class NodeBlock : public Node
{
public:
	NodeBlock();
	NodeBlock(const std::string&);
	~NodeBlock();
	Node* addChild(const Directives&, const std::string&);
	const std::vector<NodeBlock*> getChilds() const;
	const std::vector<NodeDirective*> getDirectives() const;
	
private:
	std::vector<NodeDirective*> directives;
	std::vector<NodeBlock*> nodes;
};

#endif