/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:39:32 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/17 14:25:02 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <deque>
#include <unistd.h>
#include <iomanip>
#include <sys/types.h>
#include <stdexcept>
#include <cstring>
#include <sys/wait.h>
#include <cerrno>
#include <stdlib.h>
#include <map>
#include "logtime.hpp"

#ifndef CGI_H
# define CGI_H

/*CGi norm (RFC 3875)
| Name                  | Description                                      | Example                            |
| --------------------- | ------------------------------------------------ | ---------------------------------- |
| **`REQUEST_METHOD`**  | HTTP use method                                  | `GET`, `POST`, `HEAD`, `PUT`, etc. |
| **`SCRIPT_NAME`**     | CGI root path script                             | `/cgi-bin/mon_script.cgi`          |
| **`SERVER_NAME`**     | Domain name or IP address server                 | `example.com`                      |
| **`SERVER_PORT`**     | Request TCP Port                                 | `80`, `443`, etc.                  |
| **`SERVER_PROTOCOL`** | Protocol HTTP version                            | `HTTP/1.1`, `HTTP/2.0`             |
| **`SERVER_SOFTWARE`** | Web server identifiant                           | `nginx/1.24.0`, `Apache/2.4.58`    |
*/

class CGI
{
public:
    ~CGI();
    CGI(std::string&, std::string&);  //interpreter, script
    CGI(std::string&);                //binary

    void writeBody(std::pair<char*, ssize_t>&) const;
    std::string readBody() const;

private:
    void cgi(char**, char**) const;
    void server() const;
    int buildChild(char**, char**);

private:
    pid_t   m_id_cgi;
    int     m_pipe_in[2];
    int     m_pipe_out[2];
    std::map<std::string, std::string> m_env;
};
#endif