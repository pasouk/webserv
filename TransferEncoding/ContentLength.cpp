/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ContentLength.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:12:44 by fabrice           #+#    #+#             */
/*   Updated: 2026/01/08 14:35:22 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ContentLength.hpp"

ContentLength::ContentLength() {}
ContentLength::ContentLength(const ContentLength& other) : TransferEncoding(other) {}
ContentLength::~ContentLength() {}

loadType ContentLength::loadBody1(char* buffer, ssize_t& n, s_query*& q, ssize_t& i, bool& bDelete)
{
    std::pair<char*, ssize_t> chunk;
    loadType lType = LOAD_CONTINUE;

    if (q->bodySize)
    {
        if (q->bodySize < n)
        {
            chunk = removeChunk(buffer, q->bodySize);
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
                lType = LOAD_CALL_HOOK;
            i += n;
        }
    }
    return (lType);
}

loadType ContentLength::loadBody2(char* buffer, ssize_t& n, s_query*& q, ssize_t& i)
{
    std::pair<char*, ssize_t> chunk;
    loadType lType = LOAD_CONTINUE;

    if (q->bodySize)
    {
        if (++i < n)
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
                    lType = LOAD_CALL_HOOK;
                i = n;
            }
        }
    }
    else
        lType = LOAD_CALL_HOOK;
    return (lType);
}