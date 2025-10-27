/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:38:11 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/27 13:13:04 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"
#include "Webserv.hpp"

CGI::~CGI()
{
    closeFDS();
    deleteEnvp();
}

CGI::CGI(std::string interpreter, std::string script, std::map<std::string, std::string>& env)
    : m_envp(NULL), m_id_cgi(-1)
{    
    const char* argv[3] = {interpreter.data(), script.data(), NULL};
    initFDS();
    setEnvp(env);
    if (buildChild(const_cast<char**>(argv), m_envp, m_poll))
    {
        closeFDS();
        deleteEnvp();
        throw std::runtime_error(std::strerror(errno));
    }
}

CGI::CGI(std::string binary, std::map<std::string, std::string>& env) : m_envp(NULL), m_id_cgi(-1)
{
    const char* argv[2] = {binary.data(), NULL};
    initFDS();
    setEnvp(env);
    if (buildChild(const_cast<char**>(argv), m_envp, m_poll))
    {
        closeFDS();
        deleteEnvp();
        throw std::runtime_error(std::strerror(errno));
    }
}

void CGI::setEnvp(std::map<std::string, std::string> env)
{
    std::map<std::string, std::string>::const_iterator it;
    std::string var;
    int i;

    m_envp = new (std::nothrow)char*[env.size() + 1];
    if (m_envp == NULL)
    {
        closeFDS();
        throw std::bad_alloc();
    }
    i = -1;
    for (it = env.begin(); it != env.end(); ++it)
    {
        var = (*it).first + "=" + (*it).second;
        m_envp[++i] = new (std::nothrow)char[var.length() + 1];
        if (m_envp[i] == NULL)
        {
            while (--i >= 0)
                delete [](m_envp[i]);
            delete [](m_envp);
            throw std::bad_alloc();
        }
        var.copy(m_envp[i], var.length());
        m_envp[i][var.length()] = '\0';
        //std::cout << "++++++++++" << m_envp[i] << "********\n";
    }
    m_envp[++i] = NULL;
}

void CGI::deleteEnvp()
{
    int i;

    if (m_envp == NULL)
        return ;
    i = -1;
    while (m_envp[++i] != NULL)
        delete [](m_envp[i]);
    delete [](m_envp);
}

void CGI::initFDS()
{
    m_pipe_in[0] = -1;
    m_pipe_in[1] = -1;
    m_pipe_out[0] = -1;
    m_pipe_out[1] = -1;
}

void CGI::closeFDS()
{
    if (m_pipe_in[0] != -1)
        close(m_pipe_in[0]);
    if (m_pipe_out[1] != -1)
        close(m_pipe_out[1]); 
    if (m_pipe_in[1] != -1)
        close(m_pipe_in[1]);
    if (m_pipe_out[0] != -1)
        close(m_pipe_out[0]);        
}

void CGI::writeCGI(std::pair<char*, ssize_t>& chunk) const
{
    std::ostringstream oss;
    ssize_t written, total, n;

    written = 0;
    total = chunk.second;
    while (written < total)
    {
        n = write(m_pipe_out[1], chunk.first + written, total - written);
        if (n == -1)
        {
            oss << std::strerror(errno);
            logErrMessage(oss);
            break;
        }
        written += n;
    }
}

int CGI::readCGI(std::string& response) const
{
    std::ostringstream oss;
    static char buff[2];
    int n;
    
    while ((n = read(m_pipe_in[0], buff, 2)) > 0)
    {
        buff[n] = '\0';
        response += buff;
    }
    if (n == - 1)
    {
        oss << std::strerror(errno);
        logErrMessage(oss);
    }
    return (n);
}

int CGI::buildChild(char* argv[], char* envp[], pollfd (&poll)[2])
{
    std::ostringstream oss;
    
    if (pipe(m_pipe_in) == -1 || pipe(m_pipe_out) == -1)
        return  (1);
    if (fcntl(m_pipe_in[0], F_SETFL, O_NONBLOCK) == -1 || fcntl(m_pipe_out[1], F_SETFL, O_NONBLOCK) == -1)
    {
        oss << std::strerror(errno);
	    logErrMessage(oss);
    }
    poll[0].fd = m_pipe_out[1];
    poll[0].events = POLLOUT;
    poll[0].revents = 0;
    poll[1].fd = m_pipe_in[0];
    poll[1].events = POLLIN;
    poll[1].revents = 0;
    m_id_cgi = fork();
    if (m_id_cgi == 0)
    {
        cgi(argv, envp);
        return (1);
    }
    else if (m_id_cgi == -1)
    {
        closeFDS();
        return (1);
    }
    close(m_pipe_in[1]);
    close(m_pipe_out[0]);
    return (0); 
}

void CGI::cgi(char* argv[], char* envp[])
{
    std::ostringstream oss;

    dup2(m_pipe_in[1], STDOUT_FILENO);
    dup2(m_pipe_out[0], STDIN_FILENO);
    closeFDS();
    initFDS();
    execve(argv[0], argv, envp);
    oss << argv[0] << ": " << std::strerror(errno) << std::endl;
    logErrMessage(oss);
}

const pollfd* CGI::getPoll() const
{
    return (&m_poll[0]);
}