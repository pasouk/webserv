/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logtime.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:33:02 by fabrice           #+#    #+#             */
/*   Updated: 2025/10/13 14:23:38 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "logtime.hpp"

std::string currentDateTime()
{
    std::time_t now = std::time(NULL);
    std::tm *ltm = std::localtime(&now); // convertit en heure locale
    std::ostringstream oss;
    oss << '['
        << (1900 + ltm->tm_year) << '-'
        << std::setw(2) << std::setfill('0') << (1 + ltm->tm_mon) << '-'
        << std::setw(2) << std::setfill('0') << ltm->tm_mday << ' '
        << std::setw(2) << std::setfill('0') << ltm->tm_hour << ':'
        << std::setw(2) << std::setfill('0') << ltm->tm_min << ':'
        << std::setw(2) << std::setfill('0') << ltm->tm_sec
        << ']';
    return oss.str();
}

void logMessage(std::ostringstream& oss)
{
    std::cout << currentDateTime() << " " << oss.str() << std::endl;
    oss.str("");
    oss.clear();
}
