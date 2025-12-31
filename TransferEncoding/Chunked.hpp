/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:16 by fabrice           #+#    #+#             */
/*   Updated: 2025/12/31 13:40:39 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNKED_HPP
# define CHUNKED_HPP

# include "TransferEncoding.hpp"

class Chunked : public TransferEncoding
{
public:
	Chunked();
	Chunked(const Chunked&);
	~Chunked();

	Chunked& operator=(const Chunked&);
};

#endif