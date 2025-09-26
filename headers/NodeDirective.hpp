/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:16 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/09/26 09:17:23 by fabricebuyl      ###   ########.fr       */
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
	int getClientBufferSize(size_t&) const;
	int getClientsTimeout(size_t&) const;
};

#endif