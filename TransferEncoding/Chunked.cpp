/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:14 by fabrice           #+#    #+#             */
/*   Updated: 2026/02/15 14:29:10 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunked.hpp"

Chunked::Chunked(ssize_t body_size, const std::string& temp_path)
    : TransferEncoding(body_size, temp_path), m_bItsHead(true) {}
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
    (void)n;
    static int cpt, numChunk;
    static std::ofstream ofs;   
    std::ostringstream oss;
    static std::pair<char*, ssize_t> chunk;
    std::deque<std::pair<char*, ssize_t> >::iterator it_chunk;
    loadType lType = LOAD_CONTINUE;
    std::stringstream ss;

    if (m_bItsHead)
    {
        m_header += buffer[i];
        if (m_header.find("\r\n") != std::string::npos)
        {
            ss << m_header;
            ss >> std::hex >> chunk.second;
            if (m_body_size > m_client_body_size)
            {
                if (q->bodyFile.empty())
                {
                    oss << std::setw(8) << std::setfill('0') << ++cpt;
                    q->bodyFile = m_temp_path + "/_" + oss.str();
                    oss.str("");
                    oss.clear();
                    ofs.open(q->bodyFile.c_str(), std::ios::binary);
                    if (!ofs.is_open())
                    {
                        oss << " fail to create: " << q->bodyFile << " tempory file" << std::endl;
                        logErrMessage(oss);  
                    }
                    else
                        for(it_chunk = q->bodyChunks.begin(); it_chunk != q->bodyChunks.end();)
                        {
                            ofs.write(it_chunk->first, it_chunk->second);
                            delete [](it_chunk->first);
                            it_chunk = q->bodyChunks.erase(it_chunk);
                        }
                }
            }
            oss << "client fd:" << q->fd << ", chunk n°:" << ++numChunk << " added";
            if (!q->bodyFile.empty())
                oss << " to " << q->bodyFile;
            else
                oss << " to memory";
            logOutMessage(oss);  
            m_header = "";
            m_bItsHead = false;
        }
    }
    else
    {
        m_data += buffer[i];
        if (m_data.find("\r\n") != std::string::npos)
        {
            if (chunk.second == 0)
            {
                numChunk = 0;
                m_data = "";
                m_bItsHead = true;
                ofs.close();
                lType = LOAD_CALL_HOOK;
            }
            else
            {
                chunk = removeChunk(const_cast<char*>(m_data.c_str()), chunk.second);
                if (q->bodyFile.empty())
                    q->bodyChunks.push_back(chunk);
                else
                {
                    ofs.write(chunk.first, chunk.second);
                    delete [](chunk.first);
                }
                m_data = "";
                m_bItsHead = true;
            }
        }        
    }
    return (lType);
}