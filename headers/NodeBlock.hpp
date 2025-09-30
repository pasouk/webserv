/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeBlock.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:49:51 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/30 07:26:17 by fbuyl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODEBLOCK_HPP
# define NODEBLOCK_HPP

# include "Node.hpp"

class NodeDirective;
class NodeBlock : public Node
{
public:
	NodeBlock();
	NodeBlock(const std::string&, const std::string&);
	~NodeBlock();

	NodeBlock& operator=(const NodeBlock&);

	Node* addChild(const Directives&, const std::string&);
	const std::vector<NodeBlock*> getBlocks() const;
	const std::vector<NodeDirective*> getDirectives() const;
	
private:
	std::vector<NodeDirective*> m_directives;
	std::vector<NodeBlock*> m_blocks;
};

#endif