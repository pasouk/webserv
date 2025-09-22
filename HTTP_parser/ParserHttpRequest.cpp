#include "ParserHttpRequest.hpp"
#include <sstream>

MethodMap methods_map[] = {
    {"GET", GET},
    {"POST", POST},
    {"DELETE", DELETE_},
    {"UNKNOWN",UNKNOWN}
};

bool    ParserHttpRequest::isSeveralLines()
{
    if(_rawRequest.find("\r\n") == std::string::npos)
        return false;
    return true;
}

bool    ParserHttpRequest::isBodySeparator()
{
    if(_rawRequest.find("\r\n\r\n") == std::string::npos)
        return false;   
    return true;   
}

int    ParserHttpRequest::basicChecks()
{
    if(_rawRequest.empty())
        return 400;
    if (!isSeveralLines() || !isBodySeparator())
        return 400;
    return 0;
}

parsingState operator++(parsingState &state, int)
{
    parsingState old = state;
    if (state != FINISHED)
        state = static_cast<parsingState>(state + 1);
    return old;
}

ParserHttpRequest::ParserHttpRequest(std::string rawRequest) : _rawRequest(rawRequest)
{
    _state++;
}

std::string ParserHttpRequest::getMethodLine()
{
    return _methodLine;
}

std::string ParserHttpRequest::getHeaderLine()
{
    return _headerLine;
}

std::string ParserHttpRequest::getBodyLine()
{
    return _bodyLine;
}

std::string ParserHttpRequest::getBodyLine() const
{
    return _bodyLine;
}

HttpMethod ParserHttpRequest::getMethod() const
{
    return _method;
}

std::string ParserHttpRequest::getPath() const
{
    return _path;
}

std::string ParserHttpRequest::getVersion() const
{
    return _version;
}

const std::map<std::string, std::string>& ParserHttpRequest::getHeaders() const
{
    return _headers;
}

void ParserHttpRequest::devideRequest()
{
    size_t pos1 = _rawRequest.find("\r\n");
    if (pos1 != std::string::npos)
        _methodLine = _rawRequest.substr(0, pos1);
    else
        _methodLine = "";

    size_t pos2 = _rawRequest.find("\r\n\r\n", pos1 + 2);
    if (pos2 != std::string::npos && pos2 + 4 <= _rawRequest.size())
        _headerLine = _rawRequest.substr(pos1 + 2, pos2 - (pos1 + 2));
    else
        _headerLine = "";

    if (pos2 != std::string::npos && pos2 + 4 < _rawRequest.size())
        _bodyLine = _rawRequest.substr(pos2 + 4);
    else
        _bodyLine = "";
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
        std::cout << "Invalid HHTP Request : Unknown version !\n";
        return 400;
    }
    return 0;
    
}

int    ParserHttpRequest::parseMethodLine()
{
    if (ft_countwords(_methodLine) !=3)
    {
        std::cout << "Invalid HTTP Request : Missing information in the Method line ! \n";
        return 400;
    }
    findMethod();
    findPath();
    _version = getMethodLine();
    return checkVersionAndMethod();
}

int ParserHttpRequest::parseHeaderLine()
{
    std::istringstream stream(_headerLine);
    std::string line;

    while (std::getline(stream, line, '\n'))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        size_t pos = line.find(':');
        if (pos == std::string::npos)
        {
            std::cout << "Invalid HTTP request: Missing ':' in header line!\n";
            return 400;
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key.empty())
        {
            std::cout << "Invalid HTTP request: Missing key in header line!\n";
            return 400;
        }
        if (value.empty())
        {
            std::cout << "Invalid HTTP request: Missing value in header line!\n";
            return 400;
        }

        if (!_headers.insert(std::make_pair(key, value)).second)
        {
            std::cout << "Invalid HTTP request : Duplicate header detected: " << key << "\n";
            return 400;
        }
    }

    return 0;
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

int ParserHttpRequest::parseRequest()
{
    int ret;
    ret = basicChecks();
    if (ret)
        return ret;
    devideRequest();
    ret = parseMethodLine();
    if (ret)
        return ret;
    ret = parseHeaderLine();
    if (ret)
        return ret;
    sanitize();
    return 0;
}

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