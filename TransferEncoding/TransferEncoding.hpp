/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TransferEncoding.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 11:07:04 by fabrice           #+#    #+#             */
/*   Updated: 2025/12/29 14:37:44 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRANSFERENCODING_HPP
# define TRNSFERENCODING_HPP

enum encodingType
{
    ENCODING_CHUNKED, 
    ENCODING_DEFLATE,
    ENCODING_GZIP,
    ENCODING_COMPRESS,
    ENCODING_NONE   //ENCODING_NONE means using Content-Length if there 
};

class TransferEncoding
{
public:
	TransferEncoding();
	TransferEncoding(const TransferEncoding&);
	virtual ~TransferEncoding();

	TransferEncoding& operator=(const TransferEncoding&);

    const encodingType& getType();

private:
    encodingType type;
};

#endif