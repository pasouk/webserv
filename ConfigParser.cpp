/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 13:06:40 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/17 13:42:22 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& file)
{
	//if failed -> exception
	openFile(file);
}

ConfigParser::~ConfigParser()
{
	m_config_file.close();
}

void ConfigParser::openFile(const std::string& file)
{
	m_config_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		m_config_file.open(file, std::ifstream::in);
	}
	catch(const std::ios_base::failure& e)
	{
		throw std::runtime_error(file + ": " + e.what());
	}	
}

void ConfigParser::checkLines(const std::ifstream& s)
{
	std::string l;
	while (std::getline(m_config_file, l))
	{
		
	}
}