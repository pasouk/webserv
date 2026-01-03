/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:14 by fabrice           #+#    #+#             */
/*   Updated: 2026/01/03 15:06:34 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunked.hpp"

Chunked::Chunked() : m_bItsHead(false) {}
Chunked::Chunked(const Chunked& other) : TransferEncoding(other) {}
Chunked::~Chunked() {}

loadType Chunked::loadBody1(char* buffer, ssize_t& n, s_query*& q, ssize_t& i, bool& bDelete)
{
    (void)buffer;
    (void)n;
    (void)q;
    (void)i;
    (void)bDelete;

    std::pair<char*, ssize_t> chunk;
    loadType lType = LOAD_CONTINUE;

    if (q->bodySize)
    {
        if (i + q->bodySize < n)
        {
            chunk = removeChunk(&buffer[i], q->bodySize);
            q->bodyChunks.push_back(chunk);
            i += q->bodySize;
            lType = LOAD_CALL_HOOK;
        }
        else
        {
            chunk.second = n;
            chunk.first = buffer;
            q->bodyChunks.push_back(chunk);
            bDelete = false;
            q->bodySize -= n;
            if (q->bodySize == 0)
            {
                m_bItsHead = false;
                lType = LOAD_CALL_HOOK;
            }
            i += n;
        }
    }
    return (lType);
}

loadType Chunked::loadBody2(char* buffer, ssize_t& n, s_query*& q, ssize_t& i)
{
    (void)buffer;
    (void)n;
    //(void)q;
    (void)i;

    loadType lType = LOAD_CONTINUE;
    std::stringstream ss;
    //int size = 0;
    std::pair<char*, ssize_t> chunk;

    if (i < n)
    {
        if (!m_bItsHead)
        {
            m_header += buffer[i];
            if (m_header.find("\r\n") != std::string::npos)
            {
                ss << m_header;
                ss >> std::hex >> q->bodySize;
                std::cout << "HEADER: " << m_header << "size: " << q->bodySize <<  std::endl;
                m_header = "";
                m_bItsHead = true;
            }
        }
        else
        {
            if (q->bodySize)
            {
                if (i + q->bodySize < n)
                {
                    chunk = removeChunk(&buffer[i], q->bodySize);
                    q->bodyChunks.push_back(chunk);
                    i += q->bodySize - 1;
                    lType = LOAD_CALL_HOOK;
                }
                else
                {
                    chunk = removeChunk(&buffer[i], n - i);
                    q->bodyChunks.push_back(chunk);
                    q->bodySize -= n - i;
                    if (q->bodySize == 0)
                    {
                        m_bItsHead = false;
                        lType = LOAD_CALL_HOOK;
                    }
                    i = n;
                }
            }
            else
                lType = LOAD_CALL_HOOK;
        }
        /*else
        {
            m_body += buffer[i];
             if (m_body.find("\r\n") != std::string::npos)
            {
                std::cout << "BODY: " << m_body << std::endl;
                m_body = "";
                bItsHead = false;
            }           
        }*/
    }
    return (lType);
}