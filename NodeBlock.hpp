/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeBlock.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:49:51 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/23 13:57:49 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODEBLOCK_HPP
# define NODEBLOCK_HPP

# include "Node.hpp"
# include "NodeDirective.hpp"

class NodeBlock : public Node
{
public:
	NodeBlock();
	~NodeBlock();
	void setBlock(const std::string&);
	const std::string& getName() const;
	const std::vector<NodeBlock*> getChilds() const;
	const std::vector<NodeDirective*> getDirectives() const;
	NodeBlock* addChild();
	NodeDirective* addDirective(std::string);
	
private:
	std::string block;
	std::vector<NodeDirective*> directives;
	std::vector<NodeBlock*> nodes;
};

#endif