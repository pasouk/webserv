/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 12:59:12 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/19 16:43:13 by fabricebuyl      ###   ########.fr       */
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

struct Node
{
public:
	Node();
	Node(const std::string);
	virtual ~Node();

	const std::string& getType() const;
	const std::string& getArgs() const;
	void setArgs(std::string);
	
protected:
	std::string type;
	std::string args;
};

struct NodeDirective : public Node
{
public:
	NodeDirective(std::string);

private:
	std::string directive;
};

struct NodeBlock : public Node
{
public:
	NodeBlock();
	~NodeBlock();
	void setBlock(std::string);
	NodeBlock* buildChild();
	
private:
	std::string block;
	std::vector<NodeBlock*> nodes;
};

typedef std::map<std::string, std::string> conf_directive;

class ConfigParser
{
public:
	ConfigParser();
	ConfigParser(const ConfigParser&);
	ConfigParser(const std::string&);
	~ConfigParser();

	ConfigParser& operator=(const ConfigParser&);

	conf_directive getDirective();
	void getFormat(NodeBlock &node);
	void printNode(const NodeBlock&) const;

private:
	std::ifstream m_config_file;

private:
	void openFile(const std::string&);

private:
};

#endif