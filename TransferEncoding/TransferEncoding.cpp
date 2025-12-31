/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TransferEncoding.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 11:13:06 by fabrice           #+#    #+#             */
/*   Updated: 2025/12/31 13:56:21 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TransferEncoding.hpp"

TransferEncoding::TransferEncoding() {}
TransferEncoding::TransferEncoding(const TransferEncoding& other) { (void)other; }
TransferEncoding::~TransferEncoding() {}

loadType TransferEncoding::loadBody1(char*, ssize_t&, s_query*&, ssize_t&, bool&)
{
    return (LOAD_CONTINUE);
}

loadType TransferEncoding::loadBody2(char*, ssize_t&, s_query*&, ssize_t&)
{
    return (LOAD_CONTINUE);
}

std::pair<char*, ssize_t> TransferEncoding::removeChunk(char* stream, ssize_t size)
{
	std::pair<char*, ssize_t> chunk;

	chunk.first = new (std::nothrow) char[size + 1];
	if (chunk.first == NULL)
	{
		//cleanWebserv();
		throw std::bad_alloc();
	}
	memcpy(chunk.first, stream, size);
	chunk.first[size] = '\0';
	chunk.second = size;
	return (chunk);
}
