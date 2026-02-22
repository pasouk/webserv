/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 09:57:41 by fabrice           #+#    #+#             */
/*   Updated: 2026/02/22 14:05:34 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERS_HPP
# define HEADERS_HPP
//# include "CGI.hpp"
# include "ParserHttp.hpp"
# include <stdint.h>

class TransferEncoding;
class CGI;
enum locationType
{
    LOCATION_ROOT,
    LOCATION_ALIAS,
	LOCATION_PROXY_PASS,
	LOCATION_NONE
};

struct s_query
{
	s_query() : fd(-1), cgi(NULL), httpParser(NULL), bodySize(0), port(0)
		,encoding(NULL), closeconnection(false)  {}

	int							fd;
	CGI*						cgi;
	ParserHttpRequest*			httpParser;
	ssize_t						bodySize;
	uint16_t 					port;
	TransferEncoding*			encoding;
	bool						closeconnection;
	time_t						lifeTime;
	size_t						byteSent;
	std::string					host;
	std::string					hostName;
	std::string					httpRequest;
	std::string 				formatedResponse;
	std::string					bodyFile;
	std::deque<std::pair<char*, ssize_t> >	bodyChunks;
};

struct s_location
{
	s_location() : is_cgi(NULL), type(LOCATION_NONE) {}

	bool					is_cgi;
	locationType			type;
	std::string				concatOrReplace;
	std::string				by;
	std::string				max_body_size;
	std::string				cgi_pass;
	std::string				index;
	std::vector<HttpMethod>	httpMethodsAllowed;
};

struct s_server
{
	std::vector<s_location>			locations;
	std::vector<std::string>		server_names;
	std::vector<uint16_t> 			ports;
	std::vector<std::string>		hosts;
	std::string						root;
	std::string						max_body_size;
};
#endif
