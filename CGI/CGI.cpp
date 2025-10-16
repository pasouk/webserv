/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:38:11 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/16 16:19:11 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI() : m_id_cgi(-1) {}

void CGI::run(std::string& interpreter, std::string& script) const
{
    const char* argv[3] = {interpreter.data(), script.data(), NULL};
    cgi(const_cast<char**>(argv), NULL);
}

void CGI::run(std::string& binary) const
{
    const char* argv[2] = {binary.data(), NULL};
    cgi(const_cast<char**>(argv), NULL);
}

void CGI::buildChild(char* argv[], char* envp[])
{
    m_id_cgi = fork();
    if (fork() == -1)
        throw std::runtime_error(std::strerror(errno));
    if (m_id_cgi == 0)
        cgi(argv, envp);
    server();  
}

void CGI::cgi(char* argv[], char* envp[]) const
{
    (void)envp;
    int pipe_in[2];
    int pipe_out[2];

    if (pipe(pipe_in) == -1 || pipe(pipe_out))
        exit (1);
    dup2(pipe_in[0], STDIN_FILENO);
    dup2(pipe_out[1], STDOUT_FILENO);
    close(pipe_in[1]);
    close(pipe_out[0]);

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