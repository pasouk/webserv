/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TransferEncoding.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 11:07:04 by fabrice           #+#    #+#             */
/*   Updated: 2025/12/31 13:50:32 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRANSFERENCODING_HPP
# define TRANSFERENCODING_HPP

# include <sys/types.h>
# include "ParserHttp.hpp"
# include "CGI.hpp"

enum loadType
{
    LOAD_CONTINUE,
    LOAD_CALL_HOOK,
};

class TransferEncoding;
struct s_query
{
	s_query() : fd(-1), cgi(NULL), httpParser(NULL), bodySize(0), port(0)
		,encoding(NULL)  {}

	int							fd;
	CGI*						cgi;
	ParserHttpRequest*			httpParser;
	ssize_t						bodySize;
	uint16_t 					port;
	TransferEncoding*			encoding;
	time_t						lifeTime;
	size_t						byteSent;
	std::string					host;
	std::string					hostName;
	std::string					httpRequest;
	std::string 				formatedResponse;
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

class TransferEncoding
{
public:
	TransferEncoding();
	TransferEncoding(const TransferEncoding&);
	virtual ~TransferEncoding();

	TransferEncoding& operator=(const TransferEncoding&);

    virtual loadType loadBody1(char*, ssize_t&, s_query*&, ssize_t&, bool&);
    virtual loadType loadBody2(char*, ssize_t&, s_query*&, ssize_t&);
	
protected:
    std::pair<char*, ssize_t> removeChunk(char* stream, ssize_t size);
};

#endif