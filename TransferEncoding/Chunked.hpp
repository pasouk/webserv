/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:16 by fabrice           #+#    #+#             */
/*   Updated: 2026/01/07 10:46:12 by fabrice          ###   ########.fr       */
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

	loadType loadBody1(char*, ssize_t&, s_query*&, ssize_t&, bool&);
	loadType loadBody2(char*, ssize_t&, s_query*&, ssize_t&);

private:
	bool m_bItsHead;
	std::string m_header;
	std::string m_data;
};

#endif