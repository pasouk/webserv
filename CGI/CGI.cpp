/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:38:11 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/17 15:54:38 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

//In our implementation, CGI pipes are in blocking mode (unlike sockets who are O_NONBLOCK),
//that means if a CGI is running, server could wait the end of his execution.
CGI::CGI(std::string& interpreter, std::string & script)
{
    const char* argv[3] = {interpreter.data(), script.data(), NULL};
    if (buildChild(const_cast<char**>(argv), NULL))
        throw std::runtime_error(std::strerror(errno));}

CGI::CGI(std::string& binary)
{
    const char* argv[2] = {binary.data(), NULL};
    if (buildChild(const_cast<char**>(argv), NULL))
        throw std::runtime_error(std::strerror(errno));
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

int CGI::buildChild(char* argv[], char* envp[])
{
    if (pipe(m_pipe_in) == -1 || pipe(m_pipe_out))
        return  (1);
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
        cgi(argv, envp);
    server();
    return (0); 
}

void CGI::cgi(char* argv[], char* envp[]) const
{
    (void)envp;

    dup2(m_pipe_in[0], STDIN_FILENO);
    dup2(m_pipe_out[1], STDOUT_FILENO);
    close(m_pipe_in[1]);
    close(m_pipe_out[0]);

    /*char *envp[] = {
        "REQUEST_METHOD=POST",
        "CONTENT_LENGTH=13",
        "QUERY_STRING=",
        NULL
    };*/

    execve(argv[0], argv, envp);
    exit(1);
}

void CGI::server() const
{
    //int status;

    //waitpid(m_id_cgi, &status, 0);
}