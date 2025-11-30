/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:39:32 by fabrice           #+#    #+#             */
/*   Updated: 2025/11/17 11:24:20 by fabrice          ###   ########.fr       */
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
# include "utils.hpp"

# define READBUFFERSIZE 1024

class Webserv;
class CGI
{
public:
    ~CGI();
    CGI(std::string, std::string, std::map<std::string, std::string>&, int);    //script whithout shebang
    CGI(std::string, std::map<std::string, std::string>&, int);                 //binary/script (with shebang)

    int writeCGI(std::pair<char*, ssize_t>&);
    int readCGI(std::string&);
    const pollfd* getPollfd() const;
    int getFd() const;
    pid_t getPid() const;

private:
    void cgi(char**, char**);
    int buildChild(char**, char**, pollfd(&)[2]);
    void initFDS();
    void closeFDS();
    void deleteEnvp();
    void setEnvp(std::map<std::string, std::string>);

private:
    int     m_fd_client;
    char**  m_envp;
    pid_t   m_id_cgi;
    int     m_pipe_in[2];
    int     m_pipe_out[2];
    pollfd  m_poll[2];
};
#endif