/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 13:06:40 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/23 14:18:17 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& file) : m_line(1), m_brace(0), m_file(file)
{
	if (m_blockType.empty())
	{
        m_blockType.push_back("http");
        m_blockType.push_back("server");
        m_blockType.push_back("location");
	}
	if (m_directiveType.empty())
	{
        m_directiveType.push_back("root");
        m_directiveType.push_back("server_name");
        m_directiveType.push_back("listen");
		m_directiveType.push_back("index");
	}
	
	//if failed -> exception
	openFile(file);
	getFormat(m_root);
}

ConfigParser::~ConfigParser()
{
	m_config_file.close();
	std::cout << "Destructor called\n";
}

void ConfigParser::openFile(const std::string& file)
{
	m_config_file.exceptions(std::ifstream::badbit);
	try
	{
		m_config_file.open(file.c_str(), std::ifstream::in);
	}
	catch(const std::ios_base::failure& e)
	{
		throw std::runtime_error(file + ": " + e.what());
	}	
}

void ConfigParser::getFormat(NodeBlock &node)
{
	std::stringstream 	ss;
	std::string 		block, name;
	NodeBlock			*child;
	char				c, d;
	std::ostringstream 	oss;
	
	d = ';';
	while (m_config_file.get(c))
	{
		if (c == '\n')
			++m_line;
		else if (c == '{')
		{
			++m_brace;
			ss.clear();
			ss.str(block);
			ss >> name;
			checkKeyword(node, name);
			child = node.addChild();
			child->setBlock(name);
			getFormat(*child);
			block.clear();
		}
		else if (c == '}')
		{
			--m_brace;
			checkBlock(d, name);
			return ;
		}
		else if (c == ';')
		{
			ss.clear();
			ss.str(block);
			ss >> name;
			checkKeyword(*node.addDirective(name), name);
			block.clear();
		}
		else
			block += c;
		if (!std::isspace(static_cast<unsigned char>(c)))
			d = c;
	}
	checkBrace();
}

void ConfigParser::printAST(const NodeBlock& root, std::ostream& os, int& deep) const
{
	size_t i;
	std::string spaces;

	for (int j = 0; j < deep; ++j)
		spaces += "   ";
	i = -1;
	os << spaces << root.getName() << std::endl;
	const std::vector<NodeDirective*>& directives = root.getDirectives();
	for (std::vector<NodeDirective*>::const_iterator it = directives.begin(); it < directives.end(); ++it)
		os << spaces << "   ." << "\e[0;32m" << (*it)->getName() << "\e[0m" << std::endl;
	while(++i < root.getChilds().size())
		printAST(*root.getChilds()[i], os, ++deep);
	--deep;
}

void ConfigParser::displayAST(std::ostream& os) const
{
	int	level = 0;
	printAST(m_root, os, level);
}

void ConfigParser::checkKeyword(Node& node, const std::string& name)
{
	std::vector<std::string> type;
	std::ostringstream 	oss;

	type = m_directiveType;
	if (node.getType() == "block")
		type = m_blockType;
	for (std::vector<std::string>::const_iterator it = type.begin(); it != type.end(); ++it)
		if (*it == name)
			return ;
	oss.clear();
	oss << m_line;
	m_config_file.close();
	throw std::runtime_error("error: unknown directive \e[0;32m\""
		+ name + "\" \e[0;34min\e[0m " + m_file + ":" + oss.str());
}

void ConfigParser::checkBrace()
{
	if (m_brace > 0)
	{
		m_config_file.close();
		throw std::runtime_error("error: parenthensis is missing \e[0;34min\e[0m "
			+ m_file);			
	}
}

void ConfigParser::checkBlock(char d, std::string &name)
{
	std::ostringstream 	oss;

	if ((d != '}' && d != '{' && d != ';') || m_brace < 0)
	{
		oss.clear();
		oss << m_line;
		m_config_file.close();
		throw std::runtime_error("error: invalide block \e[0;32m\""
			+ name + "\" \e[0;34min\e[0m " + m_file + ":" + oss.str());
	}
}

std::ostream& operator<<(std::ostream& os, const ConfigParser& cp)
{
    cp.displayAST(os);
    return os;
}