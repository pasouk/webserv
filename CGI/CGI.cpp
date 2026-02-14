/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:38:11 by fabrice           #+#    #+#             */
/*   Updated: 2026/02/13 12:11:26 by fabrice          ###   ########.fr       */
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
    , s_query*& client, std::vector<pollfd>& pollfd, std::vector<fdType>& pipeType)
    : m_wrote(0), m_total(0), m_executed(false), m_fd_client(client->fd)
        , m_envp(NULL), m_id_cgi(-1), m_fds(pollfd), m_fdtype(pipeType), m_client(client)
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
    , s_query*& client, std::vector<pollfd>& pollfd, std::vector<fdType>& pipeType)
    : m_wrote(0), m_total(0), m_executed(false), m_fd_client(client->fd)
        , m_envp(NULL), m_id_cgi(-1), m_fds(pollfd), m_fdtype(pipeType), m_client(client)
{
    m_binary = binary;
    m_argv = new (std::nothrow) char*[2];
    if (m_argv == NULL)
         throw std::runtime_error(std::strerror(errno));       
    m_argv[0] = const_cast<char*>(m_binary.c_str());
    m_argv[1] = NULL;
    initFDS();
    setEnvp(env);
    if (buildCGI())
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

ssize_t CGI::getWrote() const
{
    return (m_wrote);
}

ssize_t CGI::getTotal() const
{
    return (m_total);
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

ssize_t CGI::getChunksSize(ssize_t pos, const std::deque<std::pair<char*, ssize_t> >& chunks) const
{
    ssize_t size; 
    s_cursor cur;

    if (chunks.size() == 0)
        return (0);
    cur = getCursor(pos, chunks);
    size = chunks[cur.index].second - cur.offset;
    for (size_t i = cur.index + 1; i < chunks.size(); ++i)
        size += chunks[i].second;
    return (size);
}

const s_cursor CGI::getCursor(ssize_t pos, const std::deque<std::pair<char*, ssize_t> >& chunks) const
{
    s_cursor cur;
    ssize_t cpt;

    cpt = 0;
    for (size_t i = 0; i < chunks.size(); ++i)
    {
        if (cpt + chunks[i].second <= pos)
            cpt += chunks[i].second;
        else
        {
            cur.index = i;
            cur.offset = cpt + chunks[i].second - pos;
            cur.offset = chunks[i].second - cur.offset;
            break;
        }
    }
    return (cur);
}

char* CGI::createBuff(ssize_t wrote, ssize_t total, ssize_t &maxSize, const std::deque<std::pair<char*, ssize_t> >& chunks) const
{
    char* buff;
    ssize_t size;
    s_cursor cur;

    size = total - wrote;
    if (size > maxSize)
        size = maxSize;
    else
        maxSize = size;
    buff = new (std::nothrow)char[size];
    if (buff == NULL)
        return (NULL);
    for (ssize_t i = 0; i < size; ++i)
    {
        cur = getCursor(wrote + i, chunks);
        buff[i] = chunks[cur.index].first[cur.offset];
    }
    return (buff);
}

int CGI::writeCGI(const std::deque<std::pair<char*, ssize_t> >& chunks)
{
    std::ostringstream oss;
    ssize_t n;
    static ssize_t maxSize;
    static int i;
    static char* buff;

    if (!m_total)
        m_total = getChunksSize(0, chunks);
    if (!m_total)
        return (close (m_pipe_out[1]), m_pipe_out[1] = -1, -2);
    if (buff == NULL)
    {
        maxSize = CGIBUFFERSIZE;
        buff = createBuff(m_wrote, m_total, maxSize, chunks);
        if (buff == NULL)
            return (close (m_pipe_out[1]), -1);
    }
    n = 0;
    while (m_wrote < m_total)
    {
        m_client->lifeTime = std::time(NULL);
        n = write(m_pipe_out[1], &buff[i], maxSize - i);
        if (n == -1)
        {
            oss << "WRITE CGI [cgi:" << m_id_cgi << "] client fd:" << m_fd_client << ", " << std::strerror(errno);
            logErrMessage(oss);
            break;
        }
        m_wrote += n;
        if (m_wrote == m_total)
        {
            i = 0;
            delete [](buff);
            buff = NULL;
            close (m_pipe_out[1]);
            m_pipe_out[1] = -1;
            n = -2;
            break;
        }
        i += n;
        if (i == maxSize)
        {
            i = 0;
            delete [](buff);
            maxSize = CGIBUFFERSIZE;
            buff = createBuff(m_wrote, m_total, maxSize, chunks);
            if (buff == NULL)
                return (close (m_pipe_out[1]), m_pipe_out[1] = -1, -1);
        }
    }
    return (n);
}

int CGI::writeCGI(const std::string& body_file)
{
    static std::ifstream ifs;
    static int i;
    static ssize_t maxSize;
    static char* buff;
    ssize_t n;
    std::streampos size;
    std::ostringstream oss;

    if (!ifs.is_open())
    {
        ifs.open(body_file.c_str(), std::ios::binary);
        if (!ifs.is_open())
        {
            oss << "fail to open: " << body_file << " tempory file" << std::endl;;
            logErrMessage(oss);  
        }
        else
        {
            ifs.seekg (0, ifs.end);
            m_total = static_cast<ssize_t>(ifs.tellg());
            ifs.seekg (0, ifs.beg);
            if (!m_total)
                return (close (m_pipe_out[1]), m_pipe_out[1] = -1, ifs.close(), -2);
        }
    }
    if (buff == NULL)
    {
        buff = new (std::nothrow)char[CGIBUFFERSIZE];
        if (buff == NULL)
            return (close (m_pipe_out[1]), -1);
        ifs.read(buff, CGIBUFFERSIZE);
        maxSize = static_cast<ssize_t>(ifs.gcount());
    }
    n = 0;
    while (m_wrote < m_total)
    {
        m_client->lifeTime = std::time(NULL);
        n = write(m_pipe_out[1], &buff[i], maxSize - i);
        if (n == -1)
        {
            oss << "WRITE CGI [cgi:" << m_id_cgi << "] client fd:" << m_fd_client << ", " << std::strerror(errno);
            logErrMessage(oss);
            break;
        }
        m_wrote += n;
        if (m_wrote >= m_total)
        {
            i = 0;
            delete [](buff);
            buff = NULL;
            close (m_pipe_out[1]);
            m_pipe_out[1] = -1;
            n = -2;
            ifs.close();
            if (remove(body_file.c_str()) != 0)
                oss << "fail to delete: " << body_file << " tempory file" << std::endl;
            else
            {
                oss << "tempory file " << body_file << " deleted";
                m_client->bodyFile = "";
            }
            logErrMessage(oss);                 
            break;
        }       
        i += n;
        if (i == maxSize)
        {
            i = 0;
            delete [](buff);
            buff = new (std::nothrow)char[CGIBUFFERSIZE];
            if (buff == NULL)
                return (close (m_pipe_out[1]), m_pipe_out[1] = -1, -1);
            ifs.read(buff, CGIBUFFERSIZE);
            maxSize = static_cast<ssize_t>(ifs.gcount());
       }
    }
    std::cout << "OUT WRITECGI: " << m_wrote << "/" << m_total << std::endl;
    return (n);
}

int CGI::readCGI()
{
    static char buff[CGIBUFFERSIZE];
    std::ostringstream oss;
    int n;
    
    while ((n = read(m_pipe_in[0], buff, CGIBUFFERSIZE)) > 0)
    {
        m_client->lifeTime = std::time(NULL);
        m_response.append(buff, n);
        m_executed = true;
    }
    if (n == -1)
    {
        oss << "READ CGI [cgi:" << m_id_cgi << "] client fd:" << m_fd_client << ", " << std::strerror(errno);
        logErrMessage(oss);
    }
    if (m_response.length() >= static_cast<size_t>(m_total) || !m_total)
    {
        n = -2;
        m_wrote = 0;
        if (m_total)
            m_response = m_response.substr(m_response.length() - getTotal(), getTotal());
    }

    std::cout << "SIZE: " << m_response.length() << std::endl;   
    
    return (n);
}

int CGI::buildCGI()
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
    close(m_pipe_in[1]);
    close(m_pipe_out[0]);
    oss << "client fd:" << m_fd_client << ", pid: " << m_id_cgi << " is started";
    logOutMessage(oss);
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