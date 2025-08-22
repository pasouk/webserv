/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 13:06:40 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/22 15:50:09 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& file)
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
	std::ostringstream 	oss;
	std::string 		block, name;
	NodeBlock			*child;
	char				c;
	int					line;
	
	line = 1;
	while (m_config_file.get(c))
	{
		if (c == '\n')
			oss << ++line;
		else if (c == '{')
		{
			ss.clear();
			ss.str(block);
			ss >> name;
			if(!checkName(node, name))
				throw std::runtime_error("Bad NAME: " + name + " at: " + oss.str());
			child = node.addChild();
			child->setBlock(name);
			getFormat(*child);
			block.clear();
		}
		else if (c == '}')
			return ;
		else if (c == ';')
		{
			ss.clear();
			ss.str(block);
			ss >> name;
			if (!checkName(*node.addDirective(name), name))
				throw std::runtime_error("Bad NAME: " + name + " at: " + oss.str());
			block.clear();
		}
		else
			block += c;
	} 
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

bool ConfigParser::checkName(Node& node, const std::string& name) const
{
	std::vector<std::string> type;

	type = m_directiveType;
	if (node.getType() == "block")
		type = m_blockType;
	for (std::vector<std::string>::const_iterator it = type.begin(); it != type.end(); ++it)
		if (*it == name)
			return (true);	
	return (false);
}

Node::Node(const std::string type) : type(type), args("") {}

Node::Node() : type(""), args("") {}

Node::~Node() {}

const std::string& Node::getType() const
{
	return (type);
}

const std::string& Node::getArgs() const
{
	return (args);
}

void Node::setArgs(std::string args)
{
	this->args = args;
}

NodeDirective::NodeDirective(std::string directive)
	: Node("directive"), directive(directive) {}

const std::string& NodeDirective::getName() const
{
	return (directive);
}
	
NodeBlock::NodeBlock() : Node("block"),  block("unknown") {}

NodeBlock::~NodeBlock()
{
	for (std::vector<NodeBlock*>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		delete (*it);
	for (std::vector<NodeDirective*>::const_iterator it = directives.begin(); it != directives.end(); ++it)
		delete (*it);
}

void NodeBlock::setBlock(const std::string& block)
{
	this->block = block;
}

const std::string& NodeBlock::getName() const
{
	return (block);
}

const std::vector<NodeBlock*> NodeBlock::getChilds() const
{
	return (nodes);
}

const std::vector<NodeDirective*> NodeBlock::getDirectives() const
{
	return (directives);
}

NodeBlock* NodeBlock::addChild()
{
	NodeBlock* newBlock;

	newBlock = new (std::nothrow) NodeBlock();
	return (nodes.push_back(newBlock), newBlock);
}

NodeDirective* NodeBlock::addDirective(std::string directive)
{
	NodeDirective* newDirective;

	newDirective = new (std::nothrow) NodeDirective(directive);
	return (directives.push_back(newDirective), newDirective);
}

std::ostream& operator<<(std::ostream& os, const ConfigParser& cp)
{
    cp.displayAST(os);
    return os;
}