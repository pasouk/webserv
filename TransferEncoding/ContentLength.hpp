/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ContentLength.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:11:14 by fabrice           #+#    #+#             */
/*   Updated: 2025/12/29 14:29:56 by fabrice          ###   ########.fr       */
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
};

#endif