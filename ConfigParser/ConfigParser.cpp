/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 13:06:40 by fabricebuyl       #+#    #+#             */
/*   Updated: 2026/01/24 12:22:10 by fabrice          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& file) : m_line(1), m_file(file)
{
	openFile(file);

	//use new to ovoid "objects slicing" (polymorphism failed !!)
	//instead passing by value ! 
	//block
	m_directives.push_back(new Http());
	m_directives.push_back(new Server());
	m_directives.push_back(new Location());
	m_directives.push_back(new Events());
	m_directives.push_back(new LimitExcept());

	//directives
	m_directives.push_back(new Alias());
	m_directives.push_back(new Root());
	m_directives.push_back(new ServerName());
	m_directives.push_back(new Listen());
	m_directives.push_back(new Index());
	m_directives.push_back(new ClientBodyBufferSize());
	m_directives.push_back(new ClientHeaderBufferSize());
	m_directives.push_back(new ClientBodyTempPath());
	m_directives.push_back(new KeepaliveTimeout());
	m_directives.push_back(new ClientMaxBodySize());
	m_directives.push_back(new Deny());
	m_directives.push_back(new CgiPass());
	
	getFormat(m_ast);
}

ConfigParser::~ConfigParser()
{
	cleanParser();
	//std::cout << "Destructor called\n";
}

void ConfigParser::openFile(const std::string& file)
{
	try
	{
		m_config_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		m_config_file.open(file.c_str(), std::ifstream::in);
		m_config_file.exceptions(std::ifstream::badbit); //to avoid EOF exception
	}
	catch(const std::ios_base::failure& e)
	{
		cleanParser();
		throw std::runtime_error(file + ": " + e.what());
	}	
}

void ConfigParser::getFormat(NodeBlock &node)
{
	std::ostringstream 	oss;
	std::string 		block;
	char				c;
	
	while (m_config_file.get(c))
	{
		if (c == '\n')
			++m_line;
		else if (c == '{')
			buildNode(true, node, block);
		else if (c == '}')
		{			
			if (!block.empty())
			{
				for (size_t i = 0; i < block.length(); ++i)
					if (!std::isspace(block[i]))
					{
						cleanParser();
						oss << m_line;
						throw std::runtime_error("error: block is not terminated by a directive \e[0;34min\e[0m "
							+ m_file + ":" + oss.str());
					}
			}
			return ;
		}
		else if (c == ';')
			buildNode(false, node, block);
		else
			block += c;
	}
}

void ConfigParser::buildNode(bool bblock, NodeBlock &node, std::string& block)
{
	std::vector<std::string> args;
	std::stringstream 	ss;
	std::string 		name, arg;
	Node				*p_node;
	Directives			directive;

	ss.clear();
	ss.str(block);
	ss >> name;
	args.clear();
	while (ss >> arg)
		args.push_back(arg);
	directive = checkDirective(args, bblock, node.getName(), name);
	p_node = node.addChild(directive, name);
	p_node->setArgs(args);
	if (bblock)
		getFormat(*((NodeBlock*)p_node));
	block.clear();
}

const std::vector<const Node*> ConfigParser::getDirectives(const std::string& name, const NodeBlock* node) const
{
	std::vector<const Node*> directives;
	if (node == NULL)
		node = &m_ast;
	return (ast(*node, directives, name), directives);
}

void ConfigParser::ast(const NodeBlock& root, std::vector<const Node*>& nodes, const std::string& name) const
{
	std::vector<NodeBlock*>::const_iterator it_b;
	std::vector<NodeDirective*>::const_iterator it_d;
	std::vector<NodeBlock*>	blocks;
	std::vector<NodeDirective*> directives;

	directives = root.getDirectives();
	for (it_d = directives.begin(); it_d != directives.end(); ++it_d)
		if ((*it_d)->getName() == name)
			nodes.push_back(*it_d);
	blocks = root.getBlocks();
	if (blocks.size())
		for (it_b = blocks.begin(); it_b != blocks.end(); ++it_b)
		{
			if ((*it_b)->getName() == name)
				nodes.push_back(*it_b);
			ast(**it_b, nodes, name);
		}
}

const Directives& ConfigParser::checkDirective(const std::vector<std::string>& args, bool block, const std::string& parent, const std::string& name)
{
	std::ostringstream 	oss;
	std::string c, bad_arg;

	oss.clear();
	oss << m_line;
	for (std::vector<Directives*>::const_iterator it = m_directives.begin(); it != m_directives.end(); ++it)
		if ((*it)->getName() == name)
		{
			if (!(*it)->isMembership(parent))
			{
				cleanParser();
				throw std::runtime_error("error: \e[0;32m\"" + name
					+ "\"\e[0m directive is not allowed here \e[0;34min\e[0m "
					+ m_file + ":" + oss.str());
			}
			else if (block != (*it)->isBlock())
			{
				c =";";
				if ((*it)->isBlock())
					c = "}";
				cleanParser();
				throw std::runtime_error("error: \e[0;32m\"" + name
					+ "\"\e[0m directive is not terminated by \e[0;32m\""
					+ c + "\" \e[0;34min\e[0m "
					+ m_file + ":" + oss.str());
			}
			else if (args.size() > (*it)->getMaxArgs() || args.size() < (*it)->getMinArgs())
			{
				cleanParser();
				throw std::runtime_error("error: invalid number of arguments \e[0;34min \e[0;32m\""
					+ name + "\"\e[0m" + " directive \e[0;34min\e[0m "
					+ m_file + ":" + oss.str());
			}
			else if (!(*it)->areArgsValid(args, bad_arg))
			{
				cleanParser();
				throw std::runtime_error("error: invalid format arguments \e[0;34min \e[0;32m\""
					+ name + ": " + bad_arg + "\"\e[0m" + " directive \e[0;34min\e[0m "
					+ m_file + ":" + oss.str());			
			}
			return (**it);
		}
	cleanParser();
	throw std::runtime_error("error: unknown directive \e[0;32m\""
		+ name + "\" \e[0;34min\e[0m " + m_file + ":" + oss.str());
}

void ConfigParser::cleanParser()
{
	m_config_file.close();
	for (std::vector<Directives*>::const_iterator it = m_directives.begin(); it != m_directives.end(); ++it)
		delete (*it);
}