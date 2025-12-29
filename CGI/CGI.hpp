/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:39:32 by fabrice           #+#    #+#             */
/*   Updated: 2025/12/29 12:59:57 by fabrice          ###   ########.fr       */
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
# include <map>
# include <fcntl.h>
# include <poll.h>
# include <vector>
# include "utils.hpp"

# define READBUFFERSIZE 3

enum locationType
{
    LOCATION_ROOT,
    LOCATION_ALIAS,
	LOCATION_PROXY_PASS,
	LOCATION_NONE
};

enum fdType
{
	SOCKET,
	ACCEPT,
	PIPE
};

class Webserv;
class CGI
{
public:
    ~CGI();
    CGI(std::string, std::string, std::map<std::string, std::string>&
        , int, std::vector<pollfd>&, std::vector<fdType>&); //script without shebang
    CGI(std::string, std::map<std::string, std::string>&
        , int, std::vector<pollfd>&, std::vector<fdType>&); //binary/script (with shebang)
    int runCGI();
    int writeCGI(std::pair<char*, ssize_t>&);
    int readCGI();
    const pollfd* getPollfd() const;
    const std::string& getResponse() const;
    int getFd() const;
    pid_t getPid() const;
    bool wasExecuted() const;

private:
    void cgi(char**, char**);
    int buildCGI();
    void initFDS();
    void closeFDS();
    void deleteEnvp();
    void setEnvp(std::map<std::string, std::string>);
	void addPipeToPoll(std::vector<pollfd>& fds, std::vector<fdType>& fdtype);
	void removePipesFromPoll(std::vector<pollfd>& fds, std::vector<fdType>& fdtype);

private:
    bool    m_executed;
    int     m_fd_client;
    char**  m_envp;
    pid_t   m_id_cgi;
    std::vector<pollfd>& m_fds;
    std::vector<fdType>& m_fdtype;
    int     m_pipe_in[2];
    int     m_pipe_out[2];
    pollfd  m_poll[2];
    char**  m_argv;
    std::string m_binary;
    std::string m_script;
    std::string m_response;
};
#endif