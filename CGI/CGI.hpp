/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:39:32 by fabrice           #+#    #+#             */
/*   Updated: 2026/02/14 11:48:08 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CGI_H
# define CGI_H

# include <deque>
# include <unistd.h>
# include <iomanip>
# include <sys/types.h>
# include <stdexcept>
# include <cstring>
# include <sys/wait.h>
# include <cerrno>
# include <stdlib.h>
# include <limits.h>    // [CHANGED] PATH_MAX for realpath() in cgi() child process
# include <map>
# include <fcntl.h>
# include <poll.h>
# include <vector>
# include <fstream>     // [CHANGED] std::ifstream for per-instance file-write state (m_write_ifs)
# include "utils.hpp"
# include "headers.hpp"

# define CGIBUFFERSIZE 65536  // [CHANGED] Raised from 8192 → 65536 for better write throughput on large POST bodies

enum fdType
{
	SOCKET,
	ACCEPT,
	PIPE
};

struct s_cursor
{
    s_cursor() : index(0), offset(0) {}
    size_t index;
    ssize_t offset;
};

class Webserv;
class CGI
{
public:
    ~CGI();
    CGI(std::string, std::string, std::map<std::string, std::string>&
        , s_query*&, std::vector<pollfd>&, std::vector<fdType>&); //script without shebang
    CGI(std::string, std::map<std::string, std::string>&
        , s_query*&, std::vector<pollfd>&, std::vector<fdType>&); //binary/script (with shebang)
    int runCGI();
    int writeCGI(const std::deque<std::pair<char*, ssize_t> >&);
    int writeCGI(const std::string&);
    int readCGI();
    const pollfd* getPollfd() const;
    const std::string& getResponse() const;
    int getFd() const;
    pid_t getPid() const;
    bool wasExecuted() const;
    ssize_t getWrote() const;
    ssize_t getTotal() const;

private:
    void cgi(char**, char**);
    int buildCGI();
    void initFDS();
    void closeFDS();
    void deleteEnvp();
    void setEnvp(std::map<std::string, std::string>);
	void addPipeToPoll(std::vector<pollfd>& fds, std::vector<fdType>& fdtype);
	void removePipesFromPoll(std::vector<pollfd>& fds, std::vector<fdType>& fdtype);
    ssize_t getChunksSize(ssize_t, const std::deque<std::pair<char*, ssize_t> >&) const;
    const s_cursor getCursor(ssize_t, const std::deque<std::pair<char*, ssize_t> >&) const;
    char* createBuff(ssize_t, ssize_t, ssize_t&, const std::deque<std::pair<char*, ssize_t> >&) const;
    
private:
    ssize_t m_wrote;
    ssize_t m_total;
    bool    m_executed;
    int     m_fd_client;
    char**  m_envp;
    pid_t   m_id_cgi;
    std::vector<pollfd>& m_fds;
    std::vector<fdType>& m_fdtype;
    s_query* m_client;
    int     m_pipe_in[2];
    int     m_pipe_out[2];
    pollfd  m_poll[2];
    char**  m_argv;
    std::string m_binary;
    std::string m_script;
    std::string m_response;
    // [CHANGED] Per-instance write state (replaces static locals in writeCGI — static was shared across all CGI instances, causing corruption)
    char*           m_write_buff;
    int             m_write_i;
    ssize_t         m_write_maxSize;
    std::ifstream   m_write_ifs;
    size_t          m_write_chunk_idx;
    ssize_t         m_write_chunk_off;
};
#endif