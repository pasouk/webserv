/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 13:06:40 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/24 14:53:42 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& file) : m_line(1), m_brace(0), m_file(file)
{
	//block
	m_directives.push_back(Http());
	m_directives.push_back(Server());
	m_directives.push_back(Location());

	//directives
	m_directives.push_back(Root());
	m_directives.push_back(ServerName());
	m_directives.push_back(Listen());
	m_directives.push_back(Index());
	
	//if failed -> exception
	openFile(file);
	getFormat(m_ast);
}

ConfigParser::~ConfigParser()
{
	m_config_file.close();
	//std::cout << "Destructor called\n";
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
	Node				*p_node;
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
			p_node = node.addChild();
			((NodeBlock*)p_node)->addBlock(checkDirective(node.getName(), name));
			p_node->setArgs(ss);
			getFormat(*((NodeBlock*)p_node));
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
			p_node = node.addDirective(checkDirective(node.getName(), name));
			p_node->setArgs(ss);
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
	std::vector<std::string>::const_iterator it2;
	std::vector<std::string> args;
	
	for (int j = 0; j < deep; ++j)
		spaces += "   ";
	i = -1;
	os << spaces << root.getName();
	args = root.getArgs();
	for (it2 = args.begin(); it2 != args.end(); ++it2)
		os << " \e[0;33m" << *it2;
	os << "\e[0m" << std::endl;
	const std::vector<NodeDirective*>& directives = root.getDirectives();
	for (std::vector<NodeDirective*>::const_iterator it = directives.begin(); it < directives.end(); ++it)
	{
		os << spaces << "   ." << "\e[0;32m" << (*it)->getName() << "\e[0m";
		args = (*it)->getArgs();
		for (it2 = args.begin(); it2 != args.end(); ++it2)
			os << " \e[0;33m" << *it2;
		os << "\e[0m" << std::endl;
	}
	while(++i < root.getChilds().size())
		printAST(*root.getChilds()[i], os, ++deep);
	--deep;
}

void ConfigParser::displayAST(std::ostream& os) const
{
	int	level = 0;
	printAST(m_ast, os, level);
}

const Directives& ConfigParser::checkDirective(const std::string& parent, const std::string& name)
{
	std::ostringstream 	oss;

	oss.clear();
	oss << m_line;
	for (std::vector<Directives>::const_iterator it = m_directives.begin(); it != m_directives.end(); ++it)
		if ((*it).getName() == name)
		{
			if (!(*it).isValid(parent))
			{
				m_config_file.close();
				throw std::runtime_error("error: \e[0;32m\"" + name
					+ "\"\e[0m directive is not allowed here \e[0;34min\e[0m "
					+ m_file + ":" + oss.str());
			}
			return (*it);
		}
	m_config_file.close();
	throw std::runtime_error("error: unknown directive \e[0;32m\""
		+ name + "\" \e[0;34min\e[0m " + m_file + ":" + oss.str());
}

void ConfigParser::checkBrace()
{
	if (m_brace > 0)
	{
		m_config_file.close();
		throw std::runtime_error("error: brace(s) is/are missing \e[0;34min\e[0m "
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