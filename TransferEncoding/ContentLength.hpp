/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ContentLength.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:11:14 by fabrice           #+#    #+#             */
/*   Updated: 2026/01/01 14:26:37 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONTENTLENGTH_HPP
# define CONTENTLENGTH_HPP

# include "TransferEncoding.hpp"

class ContentLength : public TransferEncoding
{
public:
	ContentLength();
	ContentLength(const ContentLength&);
	~ContentLength();

	ContentLength& operator=(const ContentLength&);

   	loadType loadBody1(char*, ssize_t&, s_query*&, ssize_t&, bool&);
	loadType loadBody2(char*, ssize_t&, s_query*&, ssize_t&);
};

#endif