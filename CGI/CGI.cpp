/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:38:11 by fabrice           #+#    #+#             */
/*   Updated: 2025/11/23 13:29:18 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"
#include "Webserv.hpp"

CGI::~CGI()
{
    int status;
    std::ostringstream oss;

    removePipesFromPoll(m_fds, m_fdtype);
    closeFDS();
    deleteEnvp();
    delete [](m_argv);
    if (m_id_cgi != -1)
    {
        kill(m_id_cgi, SIGTERM);
        waitpid(m_id_cgi, &status, 0);
        oss << "Terminated CGI by server, pid: " << m_id_cgi;
        logOutMessage(oss);
    }
}

CGI::CGI(std::string binary, std::string script, std::map<std::string, std::string>& env
    , int fd, std::vector<pollfd>& pollfd, std::vector<fdType>& pipeType)
    : m_executed(false), m_fd_client(fd), m_envp(NULL), m_id_cgi(-1), m_fds(pollfd), m_fdtype(pipeType)
{  
    m_binary = binary;
    m_script = script;
    m_argv = new (std::nothrow) char*[3];
    if (m_argv == NULL)
         throw std::runtime_error(std::strerror(errno));       
    m_argv[0] = const_cast<char*>(m_binary.c_str());
    m_argv[1] = const_cast<char*>(m_script.c_str());
    m_argv[2] = NULL;
    initFDS();
    setEnvp(env);
    if (buildCGI())
    {
        delete [](m_argv);
        throw std::runtime_error(std::strerror(errno));
    }
}

CGI::CGI(std::string binary, std::map<std::string, std::string>& env
    , int fd, std::vector<pollfd>& pollfd, std::vector<fdType>& pipeType)
    : m_executed(false), m_fd_client(fd), m_envp(NULL), m_id_cgi(-1), m_fds(pollfd), m_fdtype(pipeType)
{
    m_binary = binary;
    m_argv = new (std::nothrow) char*[2];
    if (m_argv == NULL)
         throw std::runtime_error(std::strerror(errno));       
    m_argv[0] = const_cast<char*>(m_binary.c_str());
    m_argv[1] = NULL;
    initFDS();
    setEnvp(env);
    if (buildCGI(/*pollfd, pipeType)*/))
    {
        delete [](m_argv);
        throw std::runtime_error(std::strerror(errno));
    }
}

const std::string& CGI::getResponse() const
{
    return (m_response);
}

int CGI::getFd() const
{
    return (m_fd_client);
}

pid_t CGI::getPid() const
{
    return (m_id_cgi);
}

bool CGI::wasExecuted() const
{
    return (m_executed);
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
    m_envp = NULL;
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

int CGI::writeCGI(std::pair<char*, ssize_t>& chunk)
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
            oss << "[cgi:" << m_id_cgi << "] client fd:" << m_fd_client << ", " << std::strerror(errno);
            logErrMessage(oss);
            break;
        }
        written += n;
    }
    return (n);
}

int CGI::readCGI()
{
    std::ostringstream oss;
    static char buff[READBUFFERSIZE];
    int n;
    
    while ((n = read(m_pipe_in[0], buff, READBUFFERSIZE)) > 0)
    {
        buff[n] = '\0';
        m_response += buff;
        m_executed = true;
    }
    if (n == - 1)
    {
        oss << "[cgi:" << m_id_cgi << "] client fd:" << m_fd_client << ", " << std::strerror(errno);
        logErrMessage(oss);
    }
    return (n);
}

int CGI::buildCGI(/*std::vector<pollfd>& fds, std::vector<fdType>& fdtype*/)
{
    std::ostringstream oss;
    
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    if (pipe(m_pipe_in) == -1 || pipe(m_pipe_out) == -1)
        return  (1);
    if (fcntl(m_pipe_in[0], F_SETFL, O_NONBLOCK) == -1 || fcntl(m_pipe_out[1], F_SETFL, O_NONBLOCK) == -1)
    {
        oss << "[cgi] client fd:" << m_fd_client << ", " << std::strerror(errno);
	    logErrMessage(oss);
    }
    m_poll[0].fd = m_pipe_out[1];
    m_poll[0].events = POLLOUT;
    m_poll[0].revents = 0;
    m_poll[1].fd = m_pipe_in[0];
    m_poll[1].events = POLLIN;
    m_poll[1].revents = 0;
    return (0);
    /*m_id_cgi = fork();
    if (m_id_cgi == 0)
    {
        cgi(argv, envp);
        removePipesFromPoll(fds, fdtype);
        closeFDS();
        deleteEnvp();
        return (1);
    }
    else if (m_id_cgi == -1)
    {
        removePipesFromPoll(fds, fdtype);   
        closeFDS();
        deleteEnvp();
        return (1);
    }
    close(m_pipe_in[1]);
    close(m_pipe_out[0]);
    return (0);*/
}

int CGI::runCGI()
{
    std::ostringstream oss;

    addPipeToPoll(m_fds, m_fdtype);
    m_id_cgi = fork();
    if (m_id_cgi == 0)
    {
        cgi(m_argv, m_envp);
        removePipesFromPoll(m_fds, m_fdtype);
        closeFDS();
        deleteEnvp();
        return (1);
    }
    else if (m_id_cgi == -1)
    {
        removePipesFromPoll(m_fds, m_fdtype);   
        closeFDS();
        deleteEnvp();
        return (1);
    }
	oss << "client fd:" << m_fd_client << ", pid: " << m_id_cgi << " is started";
    logOutMessage(oss);
    close(m_pipe_in[1]);
    close(m_pipe_out[0]);
    return (0);    
}

void CGI::cgi(char* argv[], char* envp[])
{
    std::ostringstream oss;
    std::string fileName;
//    glob_t g;
//    int i;

    dup2(m_pipe_in[1], STDOUT_FILENO);
    dup2(m_pipe_out[0], STDIN_FILENO);
    closeFDS();
    initFDS();
/*    i = 1;
    if (argv[i] == NULL)
        i = 0;
    if (!glob(argv[i], 0, NULL, &g) && g.gl_pathc >= 1) //get the first file of the list
        argv[i] = g.gl_pathv[0];
    else
    {
        oss << argv[i] << ": " << std::strerror(errno) << std::endl;
        logErrMessage(oss);           
    }*/
    fileName = argv[0];
    argv[0] = const_cast<char*>(getFilename(fileName).data());

    /*std::cerr << "FILENAME: " << fileName << std::endl;
    for (int i = 0; argv[i] != NULL; ++i)
        std::cerr << argv[i] << std::endl;
    std::cerr << std::endl;
    for (int i = 0; envp[i] != NULL; ++i)
        std::cerr << envp[i] << std::endl;*/
        
    execve(fileName.data(), argv, envp);
    oss << fileName << ": " << std::strerror(errno) << std::endl;
    logErrMessage(oss);
}

const pollfd* CGI::getPollfd() const
{
    return (&m_poll[0]);
}

void CGI::addPipeToPoll(std::vector<pollfd>& fds, std::vector<fdType>& fdtype)
{
	fds.push_back(m_poll[0]);
	fdtype.push_back(PIPE);
	fds.push_back(m_poll[1]);
	fdtype.push_back(PIPE);
}

void CGI::removePipesFromPoll(std::vector<pollfd>& fds, std::vector<fdType>& fdtype)
{
	for (size_t i = 0; i < fds.size(); ++i)
		if (fds[i].fd == m_poll[1].fd)
		{
			fds.erase(fds.begin() + i);
			fdtype.erase(fdtype.begin() + i);
			break ;
		}
	for (size_t i = 0; i < fds.size(); ++i)
		if (fds[i].fd == m_poll[0].fd)
		{
			fds.erase(fds.begin() + i);
			fdtype.erase(fdtype.begin() + i);
			break ;
		}

}