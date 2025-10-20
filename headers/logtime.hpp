/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logtime.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:35:43 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/20 09:40:37 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGTIME_HPP
# define LOGTIME_HPP
# include <iostream>
# include <ctime>
# include <iomanip>
# include <sstream>
# include <unistd.h>

std::string currentDateTime();
void logOutMessage(std::ostringstream&);
void logErrMessage(std::ostringstream&);

#endif