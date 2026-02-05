/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TransferEncoding.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 11:13:06 by fabrice           #+#    #+#             */
/*   Updated: 2026/01/29 09:40:47 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TransferEncoding.hpp"

TransferEncoding::TransferEncoding(ssize_t body_size, const std::string& temp_path)
	: m_body_size(0), m_client_body_size(body_size), m_temp_path(temp_path) {}
TransferEncoding::TransferEncoding(const TransferEncoding& other) : m_body_size(0) { (void)other; }
TransferEncoding::~TransferEncoding() {}

ssize_t TransferEncoding::getBodySize() const
{
	return (m_body_size);
}

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
	m_body_size +=  size;
	return (chunk);
}
