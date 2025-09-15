/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:16 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/15 14:57:51 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODEDIRECTIVE_HPP
# define NODEDIRECTIVE_HPP

# include <stdint.h>
# include <sstream>
# include "Node.hpp"

class NodeBlock;
class NodeDirective : public Node
{
public:
	NodeDirective(const std::string&);
	~NodeDirective();

	NodeDirective& operator=(const NodeDirective&);

	int getListenHostPort(uint16_t&, std::string&) const;
	int getClientBodyBufferSize(size_t& size) const;
};

#endif