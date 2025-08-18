/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 13:06:40 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/18 15:17:10 by fabricebuyl      ###   ########.fr       */
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

conf_directive ConfigParser::getDirective()
{
	std::map<std::string, std::string> directives;
	std::string l, w, f;
	std::stringstream ss;

	while (std::getline(m_config_file, l, ';'))
	{
		f = "";
		ss.clear();
		ss.str(l);
		while (ss >> w)
		{
			if (f == "")
				f = w;
			else
				directives[f] += w + " ";
		}
	}
	return (directives);
}

Node ConfigParser::getFormat()
{
	Node formated;
	std::string block;
	std::stringstream ss;

	while (std::getline(m_config_file, block, '{'))
	{
		ss.clear();
		ss.str(block);

		std::cout << block << std::endl;
	}
	return (formated);
}

Node::Node(const std::string type) : type(type) {}

Node::Node() : type("") {}

Node::~Node() {}

NodeDirective::NodeDirective(std::string directive)
	: Node("directive"), directive(directive) {}
	
NodeBlock::NodeBlock(std::map<std::string, Node> block, std::string& args)
	: Node("block"), block(block), args(args) {}

const std::string& Node::getType() const
{
	return (type);
}
