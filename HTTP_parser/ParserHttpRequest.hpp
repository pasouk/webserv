#ifndef PARSERHTTPREQUEST_HPP
#define PARSERHTTPREQUEST_HPP

#include <iostream>
#include <string>
#include "Colors.hpp"
#include <map>
#include <cstring>
#include "ParserHttp.hpp"
#include <deque>

//ATTENTION A MODIFIER LE TABLEAU METHOD_MAP DANS LE .CPP SI ON MODIFIE L ENUM
enum HttpMethod
{
    GET,
    POST,
    DELETE_,
    UNKNOWN,
};

struct MethodMap 
{
    const char* name;
    HttpMethod value;
};

extern MethodMap methods_map[]; 

enum parsingState
{
    INIT,
    RAWAQUIRED,
    RAWDEVIDED,
    FINISHED
};

//parsingState operator++(parsingState &state, int);



class ParserHttpRequest
{
    private:
        std::string     _rawRequest;
        std::string     _methodLine;
        std::string     _headerLine;
        std::string     _bodyLine;

        HttpMethod                      _method;
        std::string                     _path;
        std::string                     _version;
        std::map<std::string, std::string>   _headers;

        parsingState    _state;

    public:
        ParserHttpRequest();
        ParserHttpRequest(std::string rawRequest, const std::deque<char*> &bodyChunks);

        std::string getMethodLine();
        std::string getHeaderLine();
        std::string getBodyLine();
        std::string getBodyLine() const;

        HttpMethod getMethod() const;
        std::string getPath() const;
        std::string getVersion() const;
        const std::map<std::string, std::string>& getHeaders() const;
        



        void        devideRequest();
        int        parseMethodLine();
        int        parseHeaderLine();

        void    findMethod();
        void    findPath();


        bool    isSeveralLines();
        bool    isBodySeparator();
        int     basicChecks();
        int     checkHeaderLine();
        int     checkVersionAndMethod();
        int     debugParsingRequest();
        int     parseRequest();
        void    sanitize();
        void    printParsedData();
        void    setBodyLine(const std::deque<char*> &bodyChunks);
};











#endif