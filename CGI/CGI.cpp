/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:38:11 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/19 15:12:23 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"
#include "Webserv.hpp"

CGI::~CGI()
{
    close(m_pipe_in[1]);
    close(m_pipe_out[0]);
}

CGI::CGI(std::string interpreter, std::string script, Webserv* web) : m_webserv(web)
{
    initFDS();
    const char* argv[3] = {interpreter.data(), script.data(), NULL};
    if (buildChild(const_cast<char**>(argv), NULL, m_poll))
        throw std::runtime_error(std::strerror(errno));
}

CGI::CGI(std::string binary, Webserv* web) : m_webserv(web)
{
    initFDS();
    const char* argv[2] = {binary.data(), NULL};
    if (buildChild(const_cast<char**>(argv), NULL, m_poll))
        throw std::runtime_error(std::strerror(errno));
}

void CGI::initFDS()
{
    m_pipe_in[0] = -1;
    m_pipe_in[1] = -1;
    m_pipe_out[0] = -1;
    m_pipe_out[1] = -1;
}


void CGI::writeBody(std::pair<char*, ssize_t>& chunk) const
{
    std::ostringstream oss;
    ssize_t written, total, n;

    written = 0;
    total = chunk.second;
    while (written < total)
    {
        n = write(m_pipe_in[1], chunk.first + written, total - written);
        if (n == -1)
        {
            oss << std::strerror(errno);
            logErrMessage(oss);
            break;
        }
        written += n;
    }
}

std::string CGI::readBody() const
{
    std::string response;
    
    return (response);
}

int CGI::buildChild(char* argv[], char* envp[], pollfd (&poll)[2])
{
    std::ostringstream oss;
    
    if (pipe(m_pipe_in) == -1 || pipe(m_pipe_out) == -1)
        return  (1);
    if (fcntl(m_pipe_in[1], F_SETFL, O_NONBLOCK) == -1 || fcntl(m_pipe_out[0], F_SETFL, O_NONBLOCK == -1))
    {
        oss << std::strerror(errno);
	    logErrMessage(oss);
    }
    poll[0].fd = m_pipe_out[0];
    poll[0].events = POLLIN;
    poll[0].revents = 0;
    poll[1].fd = m_pipe_in[1];
    poll[1].events = POLLOUT;
    poll[1].revents = 0;
    close(m_pipe_in[0]);
    close(m_pipe_out[1]);
    m_id_cgi = fork();
    if (fork() == -1)
    {
        close(m_pipe_in[1]);
        close(m_pipe_out[0]);
        return (1);
    }
    if (m_id_cgi == 0)
    {
        if (m_webserv != NULL)
            m_webserv->cleanWebserv();
        cgi(argv, envp);
    }
    return (0); 
}

void CGI::cgi(char* argv[], char* envp[]) const
{
    (void)envp;
    (void)argv;
    /*dup2(m_pipe_in[0], STDIN_FILENO);
    dup2(m_pipe_out[1], STDOUT_FILENO);*/
    close(m_pipe_in[0]);
    close(m_pipe_out[1]);
    close(m_pipe_in[1]);
    close(m_pipe_out[0]);

    /*char *envp[] = {
        "REQUEST_METHOD=POST",
        "CONTENT_LENGTH=13",
        "QUERY_STRING=",
        NULL
    };*/

    //execve(argv[0], argv, envp);
    //exit(1);
}

const pollfd* CGI::getPoll() const
{
    return (&m_poll[0]);
}