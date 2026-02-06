/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:16 by fabricebuyl       #+#    #+#             */
/*   Updated: 2026/02/06 10:33:06 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODEDIRECTIVE_HPP
# define NODEDIRECTIVE_HPP

# include <stdint.h>
# include <sstream>
# include "Node.hpp"
# include <sys/stat.h>
# include <sys/types.h>

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
	int getGetBodyTempFile(std::string&) const;
};

#endif