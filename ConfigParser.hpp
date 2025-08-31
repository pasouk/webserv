/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 12:59:12 by fabricebuyl       #+#    #+#             */
/*   Updated: 2025/08/31 15:02:12 by fbuyl            ###   ########.fr       */
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
# include "NodeDirective.hpp"
# include "Directives.hpp"

class ConfigParser;
std::ostream& operator<<(std::ostream& os, const ConfigParser& p);

class ConfigParser
{
public:
	ConfigParser(const ConfigParser&);
	ConfigParser();
	ConfigParser(const std::string&);
	~ConfigParser();

	ConfigParser& operator=(const ConfigParser&);

	void displayAST(std::ostream& os) const;
	const std::vector<Node*> getDirectives(const std::string&, const NodeBlock* = NULL) const;

private:
	int				m_line;
	std::string 	m_file;
	std::ifstream 	m_config_file;
	NodeBlock 		m_ast;
	std::vector<Directives*> m_directives;

private:
	void openFile(const std::string&);
	void getFormat(NodeBlock &node);
	void printAST(const NodeBlock&, std::ostream& os, int&) const;
	const Directives& checkDirective(const std::vector<std::string>&, bool, const std::string&, const std::string&);
	void buildNode(bool, NodeBlock&, std::string&);
	void ast(const NodeBlock&, std::vector<Node*>&, const std::string&) const;
	void cleanParser();
};

#endif