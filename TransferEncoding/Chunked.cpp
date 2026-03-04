/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:14 by fabrice           #+#    #+#             */
/*   Updated: 2026/03/04 10:35:07 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunked.hpp"

static int cpt = 0;

Chunked::Chunked(ssize_t body_size, const std::string& temp_path)
    : TransferEncoding(body_size, temp_path), m_bItsHead(true), m_ofs(NULL), m_chunk(NULL, 0) {}
Chunked::Chunked(const Chunked& other) : TransferEncoding(other), m_bItsHead(other.m_bItsHead), m_ofs(NULL), m_chunk(NULL, 0) {}
Chunked::~Chunked()
{
    if (m_ofs)
    {
        if (m_ofs->is_open())
            m_ofs->close();
        delete m_ofs;
    }
}

loadType Chunked::loadBody1(char* buffer, ssize_t& n, s_query*& q, ssize_t& i, bool& bDelete)
{
    (void)bDelete;
    std::ostringstream oss;
    std::deque<std::pair<char*, ssize_t> >::iterator it_chunk;
    loadType lType = LOAD_CONTINUE;
    std::stringstream ss;

    while (i < n)
    {
        if (m_bItsHead)
        {
            m_header += buffer[i];
            if (m_header.find("\r\n") != std::string::npos)
            {
                ss.clear(); ss.str("");
                ss << m_header;
                ss >> std::hex >> m_chunk.second;
                if (m_body_size > m_client_body_size)
                {
                    if (q->bodyFile.empty())
                    {
                        oss << std::setw(8) << std::setfill('0') << ++cpt;
                        q->bodyFile = m_temp_path + "/_" + oss.str();
                        oss.str("");
                        oss.clear();
                        m_ofs = new (std::nothrow) std::ofstream(q->bodyFile.c_str(), std::ios::binary);
                        if (!m_ofs || !m_ofs->is_open())
                        {
                            oss << " fail to create: " << q->bodyFile << " tempory file" << std::endl;
                            logErrMessage(oss);
                        }
                        else
                            for (it_chunk = q->bodyChunks.begin(); it_chunk != q->bodyChunks.end();)
                            {
                                m_ofs->write(it_chunk->first, it_chunk->second);
                                delete [](it_chunk->first);
                                it_chunk = q->bodyChunks.erase(it_chunk);
                            }
                    }
                }
                m_header = "";
                m_bItsHead = false;
            }
            ++i;
        }
        else
        {
            ssize_t chunk_total = m_chunk.second + 2;  // data + trailing \r\n
            ssize_t have = (ssize_t)m_data.size();
            ssize_t need = chunk_total - have;
            ssize_t avail = n - i;
            ssize_t take = (avail < need) ? avail : need;
            m_data.append(buffer + i, take);
            i += take;
            if ((ssize_t)m_data.size() >= chunk_total)
            {
                if (m_chunk.second == 0)
                {
                    { std::string tmp; m_data.swap(tmp); }
                    m_bItsHead = true;
                    if (m_ofs)
                    {
                        m_ofs->close();
                        delete m_ofs;
                        m_ofs = NULL;
                    }
                    lType = LOAD_CALL_HOOK;
                    break ;
                }
                else
                {
                    m_chunk = removeChunk(const_cast<char*>(m_data.c_str()), m_chunk.second);
                    if (q->bodyFile.empty())
                        q->bodyChunks.push_back(m_chunk);
                    else
                    {
                        m_ofs->write(m_chunk.first, m_chunk.second);
                        delete [](m_chunk.first);
                    }
                    { std::string tmp; m_data.swap(tmp); }
                    m_bItsHead = true;
                }
            }
        }
    }
    return (lType);
}

loadType Chunked::loadBody2(char* buffer, ssize_t& n, s_query*& q, ssize_t& i)
{
    (void)n;
    std::ostringstream oss;
    std::deque<std::pair<char*, ssize_t> >::iterator it_chunk;
    loadType lType = LOAD_CONTINUE;
    std::stringstream ss;

    if (m_bItsHead)
    {
        m_header += buffer[i];
        if (m_header.find("\r\n") != std::string::npos)
        {
            ss << m_header;
            ss >> std::hex >> m_chunk.second;
            if (m_body_size > m_client_body_size)
            {
                if (q->bodyFile.empty())
                {
                    oss << std::setw(8) << std::setfill('0') << ++cpt;
                    q->bodyFile = m_temp_path + "/_" + oss.str();
                    oss.str("");
                    oss.clear();
                    m_ofs = new (std::nothrow) std::ofstream(q->bodyFile.c_str(), std::ios::binary);
                    if (!m_ofs || !m_ofs->is_open())
                    {
                        oss << " fail to create: " << q->bodyFile << " tempory file" << std::endl;
                        logErrMessage(oss);
                    }
                    else
                        for(it_chunk = q->bodyChunks.begin(); it_chunk != q->bodyChunks.end();)
                        {
                            m_ofs->write(it_chunk->first, it_chunk->second);
                            delete [](it_chunk->first);
                            it_chunk = q->bodyChunks.erase(it_chunk);
                        }
                }
            }
            //oss << "client fd:" << q->fd << ", chunk n°:" << ++numChunk << " added";
            //if (!q->bodyFile.empty())
            //    oss << " to " << q->bodyFile;
            //else
            //    oss << " to memory";
            //logOutMessage(oss);
            m_header = "";
            m_bItsHead = false;
        }
    }
    else
    {
        m_data += buffer[i];
        if (m_data.find("\r\n") != std::string::npos)
        {
            if (m_chunk.second == 0)
            {
                //numChunk = 0;
                { std::string tmp; m_data.swap(tmp); }
                m_bItsHead = true;
                if (m_ofs)
                {
                    m_ofs->close();
                    delete m_ofs;
                    m_ofs = NULL;
                }
                lType = LOAD_CALL_HOOK;
            }
            else
            {
                m_chunk = removeChunk(const_cast<char*>(m_data.c_str()), m_chunk.second);
                if (q->bodyFile.empty())
                    q->bodyChunks.push_back(m_chunk);
                else
                {
                    m_ofs->write(m_chunk.first, m_chunk.second);
                    delete [](m_chunk.first);
                }
                { std::string tmp; m_data.swap(tmp); }
                m_bItsHead = true;
            }
        }
    }
    return (lType);
}
