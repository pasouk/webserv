/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logtime.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:35:43 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/13 14:24:38 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGTIME_HPP
# define LOGTIME_HPP
# include <iostream>
# include <ctime>
# include <iomanip>
# include <sstream>

std::string currentDateTime();
void logMessage(std::ostringstream&);

#endif