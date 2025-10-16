/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:39:32 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/16 16:18:51 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <deque>
#include <unistd.h>
#include <sys/types.h>
#include <stdexcept>
#include <cstring>
#include <sys/wait.h>
#include <cerrno>
#include <stdlib.h>
#include <map>

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
    CGI();

    void run(std::string&, std::string&) const;   //interpreter, script
    void run(std::string&) const ;                 //binary

private:
    void cgi(char**, char**) const;
    void server() const;
    void buildChild(char**, char**);

private:
    pid_t   m_id_cgi;
    std::map<std::string, std::string> m_env;
};
#endif