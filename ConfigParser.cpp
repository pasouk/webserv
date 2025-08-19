/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 13:06:40 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/19 16:43:33 by fabricebuyl      ###   ########.fr       */
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
	std::string 		l, w, f;
	std::stringstream	ss;

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

void ConfigParser::getFormat(NodeBlock &node)
{
	std::stringstream 	ss;
	std::string 		block, name;
	char				c;
	
	(void)node;
	m_config_file.get(c);
	while (m_config_file.get(c))
	{
		if (c == '{')
		{
			ss.clear();
			ss.str(block);
			ss >> name;		
			node.setBlock(name);
			getFormat(*node.buildChild());
			block.clear();
		}
		else
			block += c;
	} 
	/*ss.clear();
	ss.str(block);
	ss >> name;
	std::cout << name << std::endl;
	block.clear();*/
}

void ConfigParser::printNode(const NodeBlock& root) const
{
	(void)root;
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
	
NodeBlock::NodeBlock() : Node("block"), block("") {}

NodeBlock::~NodeBlock()
{
	for (std::vector<NodeBlock*>::iterator it = nodes.begin(); it != nodes.end(); ++it)
	{
		delete (*it);
		std::cout << "destruct Node\n";
	}
}

void NodeBlock::setBlock(std::string block)
{
	this->block = block;
}

NodeBlock* NodeBlock::buildChild()
{
	NodeBlock *newBlock;

	newBlock = new (std::nothrow) NodeBlock();
	return (nodes.push_back(newBlock), newBlock);
}

