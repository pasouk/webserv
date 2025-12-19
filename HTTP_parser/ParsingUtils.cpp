#include "ParserHttpRequest.hpp"
#include <sstream>

MethodMap methods_map[] = {
    {"GET", GET},
    {"POST", POST},
    {"DELETE", DELETE_},
    {"HEAD", HEAD},
    {"UNKNOWN",UNKNOWN}
};

void ParserHttpRequest::sanitize()
{
    _path    = trim(_path);
    _version = trim(_version);

    std::map<std::string, std::string> cleaned;
    for (std::map<std::string, std::string>::iterator it = _headers.begin();
         it != _headers.end(); ++it) 
    {
        std::string key = trim(it->first);
        std::string value = trim(it->second);
        cleaned[key] = value;
    }
    _headers.swap(cleaned);
}

int    ParserHttpRequest::debugParsingRequest()
{
    int ret;
    std::cout << Colors::BLUE << "\n\n----------- STEP 0 : Before parsing (Raw request) ----------\n" \
     << Colors::RESET << std::endl  << _rawRequest << std::endl;
    devideRequest();
    std::cout << Colors::BLUE << "\n\n----------- STEP 1 : devide request in 3 ---------- \n" \
     << Colors::RESET << std::endl << Colors::CYAN << "Method line : " << Colors::RESET << getMethodLine() << std::endl \
     << Colors::RESET << std::endl << Colors::CYAN << "Header line : " << Colors::RESET << getHeaderLine() << std::endl \
     << Colors::RESET << std::endl << Colors::CYAN << "body line : " << Colors::RESET << getBodyLine() << std::endl;
    ret = parseMethodLine();
    if (ret)
        return ret;
    std::cout << Colors::BLUE << "\n\n----------- STEP 2 : Parsing method line ----------\n" \
     << Colors::RESET << std::endl << Colors::CYAN << "Method found: " << Colors::RESET << methods_map[getMethod()].name << std::endl \
     << Colors::RESET << Colors::CYAN << "Path found : " << Colors::RESET << getPath() << std::endl \
     << Colors::RESET << Colors::CYAN << "Version found : " << Colors::RESET << getVersion() << std::endl;
     ret = parseHeaderLine();
     if (ret)
        return ret;
     std::cout << Colors::BLUE << "\n\n----------- STEP 3 : Parsing headers ----------\n" ;
     std::cout << Colors::CYAN << "Headers fond :  " << Colors::RESET << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = getHeaders().begin();
        it != getHeaders().end(); ++it)
    {
        std::cout << it->first << " : " << it->second << std::endl;
    }
      return 0;
}

void ParserHttpRequest::printParsedData()
{
    std::cout << Colors::BLUE << "Method : " << Colors::RESET << _method << std::endl;
    std::cout << Colors::BLUE << "Path : " << Colors::RESET << _path << std::endl;
    std::cout << Colors::BLUE << "Version : " << Colors::RESET << _version << std::endl;
    std::cout << Colors::BLUE << "Headers : " << Colors::RESET  ;
    for (std::map<std::string, std::string>::const_iterator it = getHeaders().begin();
        it != getHeaders().end(); ++it)
    {
        std::cout << it->first << " : " << it->second << std::endl;
    }
}

void ParserHttpRequest::findMethod()
{
    _method = UNKNOWN;
    size_t methodCount = sizeof(methods_map) / sizeof(methods_map[0]);
    
    for (size_t i = 0; i < methodCount; i++)
    {
        if (_methodLine.find(methods_map[i].name) != std::string::npos)
        {
            _method = methods_map[i].value;

            _methodLine = _methodLine.substr(strlen(methods_map[i].name) + 1);
            return;
        }
    }
}

void    ParserHttpRequest::findPath()
{
    size_t pos = _methodLine.find(" ");
    _path = _methodLine.substr(0, pos);
    _methodLine = _methodLine.substr(pos + 1);
}

int ParserHttpRequest::checkVersionAndMethod()
{
    if (_method == UNKNOWN)
    {
        std::cout << "Invalid HHTP Request : Unknown method !\n";
        return 400;
    }
    if (_version != "HTTP/1.0" && _version != "HTTP/1.1")
    {
        std::cout << "Invalid HHTP Request : Unknown version : " << _version << std::endl;
        return 400;
    }
    return 0;
    
}