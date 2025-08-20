/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 12:59:12 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/20 14:14:47 by fabricebuyl      ###   ########.fr       */
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

class ConfigParser;
std::ostream& operator<<(std::ostream& os, const ConfigParser& p);

struct Node
{
public:
	Node();
	Node(const std::string);
	virtual ~Node();

	virtual const std::string& getName() const = 0;
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
	const std::string& getName() const;

private:
	std::string directive;
};

struct NodeBlock : public Node
{
public:
	NodeBlock();
	~NodeBlock();
	void setBlock(std::string);
	const std::string& getName() const;
	const std::vector<NodeBlock*> getChilds() const;
	const std::vector<NodeDirective*> getDirectives() const;
	NodeBlock* addChild();
	NodeDirective* addDirective(std::string);
	
private:
	std::string block;
	std::vector<NodeDirective*> directives;
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

	void print(std::ostream& os) const;

private:
	std::ifstream m_config_file;
	NodeBlock m_root;

private:
	void openFile(const std::string&);
	void getFormat(NodeBlock &node);
	void printTree(const NodeBlock&, std::ostream& os, int&) const;
};

#endif