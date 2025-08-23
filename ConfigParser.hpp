/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 12:59:12 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/23 14:18:17 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <fstream>
# include <sstream>
# include <iostream>
# include <map>
# include <vector>
# include "NodeBlock.hpp"

class ConfigParser;
std::ostream& operator<<(std::ostream& os, const ConfigParser& p);

class ConfigParser
{
public:
	ConfigParser();
	ConfigParser(const ConfigParser&);
	ConfigParser(const std::string&);
	~ConfigParser();

	ConfigParser& operator=(const ConfigParser&);

	void displayAST(std::ostream& os) const;

private:
	int				m_line;
	int				m_brace;
	std::string 	m_file;
	std::ifstream 	m_config_file;
	NodeBlock 		m_root;
	std::vector<std::string> m_blockType;
	std::vector<std::string> m_directiveType;

private:
	void openFile(const std::string&);
	void getFormat(NodeBlock &node);
	void printAST(const NodeBlock&, std::ostream& os, int&) const;
	void checkKeyword(Node&, const std::string&);
	void checkBrace();
	void checkBlock(char, std::string&);
};

#endif