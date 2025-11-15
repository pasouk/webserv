/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:35:43 by fabrice           #+#    #+#             */
/*   Updated: 2025/11/14 08:33:14 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP
# include <iostream>
# include <ctime>
# include <iomanip>
# include <sstream>
# include <unistd.h>
# include <sys/stat.h>
# include <stdint.h>
# include <stdio.h>

bool is_elf_binary(const char *path);
bool is_macho_binary(const char *path);
bool is_executable(const char *path);
bool is_path(const char *path);
std::string getFilename(const std::string&);
std::string currentDateTime();
void logOutMessage(std::ostringstream&);
void logErrMessage(std::ostringstream&);

#endif