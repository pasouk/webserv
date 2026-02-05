/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TransferEncoding.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 11:07:04 by fabrice           #+#    #+#             */
/*   Updated: 2026/01/28 15:05:01 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRANSFERENCODING_HPP
# define TRANSFERENCODING_HPP

# include <sys/types.h>
# include "headers.hpp"

enum loadType
{
    LOAD_CONTINUE,
    LOAD_CALL_HOOK,
};

class TransferEncoding
{
public:
	TransferEncoding(ssize_t, const std::string&);
	TransferEncoding(const TransferEncoding&);
	ssize_t getBodySize() const;
	virtual ~TransferEncoding();

	TransferEncoding& operator=(const TransferEncoding&);

    virtual loadType loadBody1(char*, ssize_t&, s_query*&, ssize_t&, bool&);
    virtual loadType loadBody2(char*, ssize_t&, s_query*&, ssize_t&);
	
protected:
    std::pair<char*, ssize_t> removeChunk(char* stream, ssize_t size);

protected:
	ssize_t	m_body_size;
	ssize_t m_client_body_size;
	std::string m_temp_path;
};

#endif