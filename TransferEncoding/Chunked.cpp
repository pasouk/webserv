/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:14 by fabrice           #+#    #+#             */
/*   Updated: 2026/01/02 11:56:07 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunked.hpp"

Chunked::Chunked() {}
Chunked::Chunked(const Chunked& other) : TransferEncoding(other) {}
Chunked::~Chunked() {}

loadType Chunked::loadBody1(char* buffer, ssize_t& n, s_query*& q, ssize_t& i, bool& bDelete)
{
    (void)buffer;
    (void)n;
    (void)q;
    (void)i;
    (void)bDelete;

    loadType lType = LOAD_CONTINUE;

    return (lType);
}

loadType Chunked::loadBody2(char* buffer, ssize_t& n, s_query*& q, ssize_t& i)
{
    (void)buffer;
    (void)n;
    (void)q;
    (void)i;

    loadType lType = LOAD_CONTINUE;

    if (++i < n)
    {
        m_header += buffer[i];
        if (m_header.find("\r\n") != std::string::npos)
        {
            std::cout << "HEADER: " << m_header << std::endl;
            m_header = "";
        }

    }
    else
    {

        i = n;
    }

    return (lType);
}

const std::string Chunked::getHeadChunck() const
{
    std::string head;
    return (head);
}