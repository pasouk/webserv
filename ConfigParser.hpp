/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 12:59:12 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/18 15:11:41 by fabricebuyl      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <fstream>
# include <sstream>
# include <iostream>
# include <map>

struct Node
{
public:
	Node();
	Node(const std::string);
	virtual ~Node();
	const std::string& getType() const;
protected:
	std::string type;
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
	NodeBlock(std::map<std::string, Node>, std::string&);
private:
	std::map<std::string, Node> block;
	std::string args;
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
	Node getFormat();

private:
	std::ifstream m_config_file;

private:
	void openFile(const std::string&);

private:
};

#endif