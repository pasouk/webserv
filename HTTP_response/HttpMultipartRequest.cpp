#include "HttpMultipartRequest.hpp"

SubPartRequest::SubPartRequest(std::string rawRequest ): _rawRequest(rawRequest)
{
    
}

const std::deque<std::pair<char*, ssize_t> >& SubPartRequest::getBodyBuffer() const 
{ 
    return _bodyBuffer; 
}

std::string SubPartRequest::getHeaderLine()
{
    return _headerLine;
}

std::string SubPartRequest::getBodyLine()
{
    return _bodyLine;
}

std::string SubPartRequest::getBodyLine() const
{
    return _bodyLine;
}

const std::map<std::string, std::string>& SubPartRequest::getHeaders() const
{
    return _headers;
}

void SubPartRequest::devideRequest()
{
    size_t pos = _rawRequest.find("\r\n\r\n");

    if (pos != std::string::npos)
    {
        _headerLine = _rawRequest.substr(0, pos);
        size_t bodySize = _rawRequest.size() - (pos + 4);

        if (bodySize > 0)
        {

            _bodyLine = _rawRequest.substr(pos + 4);

            char* bodyPtr = new char[bodySize];
            memcpy(bodyPtr, _rawRequest.data() + pos + 4, bodySize);
            _bodyBuffer.push_back(std::make_pair(bodyPtr, bodySize));
        }
        else
        {
            _bodyLine.clear();
        }
    }
    else
    {
        _headerLine = _rawRequest;
        _bodyLine.clear();
    }
}



int SubPartRequest::parseHeaderLine()
{
    std::istringstream stream(_headerLine);
    std::string line;

    while (std::getline(stream, line, '\n'))
    {
        if (!line.empty() && (line[line.size() - 1] == '\r'))
            line = line.substr(0, line.size() - 1);
        if (line.empty())
            continue;
        size_t pos = line.find(':');
        if (pos == std::string::npos)
        {
            std::cout << "Warning: Missing ':' in subpart header line: " << line << "\n";
            continue;
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);
        while (!value.empty() && (value[value.size() - 1] == ' ' || value[value.size() - 1] == '\t'))
            value.erase(value.size() - 1, 1);

        if (key.empty())
        {
            std::cout << "Warning: empty key in subpart header line: " << line << "\n";
            continue;
        }
        if (value.empty())
        {
            std::cout << "Warning: empty value in subpart header line: " << line << "\n";
            continue;
        }
        //std::cout << "Inserting header => [" << key << "] = [" << value << "]\n";
        _headers[key] = value;

    }

    return 0;
}

bool    SubPartRequest::isSeveralLines()
{
    if(_rawRequest.find("\r\n") == std::string::npos)
        return false;
    return true;
}

bool    SubPartRequest::isBodySeparator()
{
    if(_rawRequest.find("\r\n\r\n") == std::string::npos)
        return false;   
    return true;   
}

int    SubPartRequest::basicChecks()
{
    if(_rawRequest.empty())
        return 400;
    if (!isSeveralLines() || !isBodySeparator())
        return 400;
    return 0;
}

void SubPartRequest::printParsedData()
{
    std::cout << Colors::GREEN<< "Headers : " << Colors::RESET  ;
    for (std::map<std::string, std::string>::const_iterator it = getHeaders().begin();
        it != getHeaders().end(); ++it)
    {
        std::cout << it->first << " : " << it->second << std::endl;
    }
    std::cout << Colors::GREEN << "Body : " << _bodyLine << std::endl << Colors::RESET  ;
    std::cout << "-------------------------------------------------\n";
}