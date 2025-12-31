/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:14 by fabrice           #+#    #+#             */
/*   Updated: 2025/12/31 13:53:49 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunked.hpp"

Chunked::Chunked() {}
Chunked::Chunked(const Chunked& other) : TransferEncoding(other) {}
Chunked::~Chunked() {}