#include "ParserHttpRequest.hpp"

MethodMap methods_map[] = {
    {"GET", GET},
    {"POST", POST},
    {"DELETE", DELETE_},
    {"UNKNOWN",UNKNOWN}
};

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

HttpMethod ParserHttpRequest::getMethod()
{
    return _method;
}

std::string ParserHttpRequest::getPath()
{
    return _path;
}

std::string ParserHttpRequest::getVersion()
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
        _methodLine = ""; // mettre une erreur + tard

    size_t pos2 = _rawRequest.find("\r\n\r\n", pos1 + 2);
    if (pos2 != std::string::npos && pos2 + 4 <= _rawRequest.size())
        _headerLine = _rawRequest.substr(pos1 + 2, pos2 - (pos1 + 2));
    else
        _headerLine = ""; //mettre une erreur + tard

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

void    ParserHttpRequest::parseMethodLine()
{
    findMethod();
    findPath();
    _version = getMethodLine();
}

void    ParserHttpRequest::parseHeaderLine()
{
    size_t column;
    size_t lineEnd;
    std::string headerName;
    std::string headerValue;

    while(!_headerLine.empty())
    {
        //std::cout << "headerline beginning :" << _headerLine << std::endl;
        column = _headerLine.find(":");
       // std::cout << "column : " <<  column << std::endl;
        if (column == std::string::npos)    
            return;
        lineEnd = _headerLine.find("\r\n");
        if (lineEnd == std::string::npos)
        {
            headerName = _headerLine.substr(0, column);
            headerValue = _headerLine.substr(column + 1);
            _headers[headerName] = headerValue;
            _headerLine.clear();
         //           std::cout << "header name :" << headerName << std::endl;
        //std::cout << "header value :" << headerValue << std::endl;
        //std::cout << "headerline end :" << _headerLine << std::endl;
            return;
        }
       // std::cout << "end : " <<  lineEnd << std::endl;
        headerName = _headerLine.substr(0, column);
        headerValue = _headerLine.substr(column + 1, lineEnd - (column + 1));
        _headers[headerName] = headerValue;
        _headerLine = _headerLine.substr(lineEnd + 2);

        
        //std::cout << "header name :" << headerName << std::endl;
        //std::cout << "header value :" << headerValue << std::endl;
        //std::cout << "headerline end :" << _headerLine << std::endl;
       // std::cout << "--------------------------------------\n\n";
    }
}


int main()
{
    std::string rawRequest = "POST / HTTP/1.1\r\n \
        Host: localhost:8080\r\n \
        User-Agent: curl/7.81.0\r\n \
        Accept: */*\r\n \
        Content-Length: 7\r\n \
        Content-Type: application/x-www-form-urlencoded\r\n\r\n name=42";

    ParserHttpRequest instance1(rawRequest);

    std::cout << Colors::BLUE << "\n\n----------- STEP 0 : Before parsing (Raw request) ----------\n" \
     << Colors::RESET << std::endl  << rawRequest << std::endl;
     
    instance1.devideRequest();
    std::cout << Colors::BLUE << "\n\n----------- STEP 1 : devide request in 3 ---------- \n" \
     << Colors::RESET << std::endl << Colors::CYAN << "Method line : " << Colors::RESET << instance1.getMethodLine() << std::endl \
     << Colors::RESET << std::endl << Colors::CYAN << "Header line : " << Colors::RESET << instance1.getHeaderLine() << std::endl \
     << Colors::RESET << std::endl << Colors::CYAN << "body line : " << Colors::RESET << instance1.getBodyLine() << std::endl;

    instance1.parseMethodLine();
        std::cout << Colors::BLUE << "\n\n----------- STEP 2 : Parsing method line ----------\n" \
     << Colors::RESET << std::endl << Colors::CYAN << "Method found: " << Colors::RESET << methods_map[instance1.getMethod()].name << std::endl \
     << Colors::RESET << Colors::CYAN << "Path found : " << Colors::RESET << instance1.getPath() << std::endl \
     << Colors::RESET << Colors::CYAN << "Version found : " << Colors::RESET << instance1.getVersion() << std::endl;


     instance1.parseHeaderLine();
     std::cout << Colors::BLUE << "\n\n----------- STEP 3 : Parsing headers ----------\n" ;
     std::cout << Colors::CYAN << "Headers fond :  " << Colors::RESET << std::endl;
for (std::map<std::string, std::string>::const_iterator it = instance1.getHeaders().begin();
     it != instance1.getHeaders().end(); ++it)
{
    std::cout << it->first << " : " << it->second << std::endl;
}
     std::cout << "rest of headerline:" << instance1.getHeaderLine() ;
     //<< Colors::RESET << std::endl << Colors::CYAN << "Method found: " << Colors::RESET << methods_map[instance1.getMethod()].name << std::endl \
     
}