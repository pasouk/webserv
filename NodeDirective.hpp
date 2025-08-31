/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeDirective.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 11:46:16 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/31 12:29:28 by fbuyl            ###   ########.fr       */
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

	void getHostPort(uint16_t&, std::string&) const;
};

#endif