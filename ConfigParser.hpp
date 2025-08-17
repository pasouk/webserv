/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 12:59:12 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/17 13:41:49 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <fstream>

class ConfigParser
{
public:
	ConfigParser();
	ConfigParser(const ConfigParser&);
	ConfigParser(const std::string&);
	~ConfigParser();

private:
	std::ifstream m_config_file;

private:
	void openFile(const std::string&);
	void checkLines(const std::ifstream&);
};

#endif