#include "ParserHttpRequest.hpp"
#include <sstream>

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

