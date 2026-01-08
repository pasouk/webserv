/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:14 by fabrice           #+#    #+#             */
/*   Updated: 2026/01/08 13:17:43 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunked.hpp"

Chunked::Chunked() : m_bItsHead(true) {}
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

    static std::pair<char*, ssize_t> chunk;
    loadType lType = LOAD_CONTINUE;
    std::stringstream ss;

    if (m_bItsHead)
    {
        m_header += buffer[i];
        if (m_header.find("\r\n") != std::string::npos)
        {
            ss << m_header;
            ss >> std::hex >> chunk.second;
            //std::cout << "HEADER: " << m_header << "size: " << chunk.second << std::endl;
            m_header = "";
            m_bItsHead = false;
        }
    }
    else
    {
        if (chunk.second == 0)
        {
            lType = LOAD_CALL_HOOK;
            std::cout << "CALL HOOK !!!\n";
        }
        m_data += buffer[i];
        if (m_data.find("\r\n") != std::string::npos)
        {
            chunk = removeChunk(const_cast<char*>(m_data.c_str()), chunk.second);
            q->bodyChunks.push_back(chunk);
            //std::cout << "DATA: " << chunk.first << std::endl;
            m_data = "";
            m_bItsHead = true;
        }        
    }
    return (lType);
}