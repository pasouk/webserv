/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:33:02 by fabrice           #+#    #+#             */
/*   Updated: 2025/11/14 08:37:43 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

bool is_executable(const char *path)
{
    struct stat st;

    if (stat(path, &st) != 0)
        return false;
    if (!S_ISREG(st.st_mode))
        return false;
    return access(path, X_OK) == 0;
}

bool is_path(const char *path)
{
    struct stat s;
    
    return stat(path, &s) == 0; 
}

std::string getFilename(const std::string& path)
{
    size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

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

void logOutMessage(std::ostringstream& oss)
{
    std::cout << currentDateTime() << " " << getpid() << " " << getppid() << " " << oss.str() << std::endl;
    oss.str("");
    oss.clear();
}

void logErrMessage(std::ostringstream& oss)
{
    std::cerr << currentDateTime() << " " << getpid() << " " << getppid() << " " << oss.str() << std::endl;
    oss.str("");
    oss.clear();
}

