/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:16 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/11/03 10:51:51 by fbuyl            ###   ########.fr       */
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
	NodeDirective(const std::string&, Node*, int);
	~NodeDirective();

	NodeDirective& operator=(const NodeDirective&);

	int getListenHostPort(uint16_t&, std::string&) const;
	int getClientSize(size_t&) const;
	int getClientsTimeout(size_t&) const;
	int getHttpMethod(size_t, HttpMethod&) const;
};

#endif