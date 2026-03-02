#include "ParserHttpRequest.hpp"
#include <sstream>



ParserHttpRequest::ParserHttpRequest(std::string rawRequest) : _rawRequest(rawRequest)
{
    _error = parseRequest();
}

ParserHttpRequest::ParserHttpRequest(std::string rawRequest, const std::deque<std::pair<char*, ssize_t> > &bodyChunks) : _rawRequest(rawRequest)
{
    setBodyLine(bodyChunks);
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

int ParserHttpRequest::getError() const
{
    return (_error);
}

void ParserHttpRequest::setError(int error)
{
    _error = error;
}

HttpMethod ParserHttpRequest::getMethod() const
{
    return _method;
}

std::string ParserHttpRequest::getPath() const
{
    return _path;
}

std::string ParserHttpRequest::getRawPath() const
{
    return _rawPath;
}

void ParserHttpRequest::setPath(std::string path)
{
    _path = path;
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
}

bool ParserHttpRequest::isCgiRequest(const std::string &path)
{
    const std::string arr[] = {".py", ".php", ".pl", ".cgi"};
    const std::vector<std::string> cgiExt(arr, arr + sizeof(arr) / sizeof(arr[0]));
    const std::string cgiDir = "/cgi-bin/";

    if (path.find(cgiDir) == 0)
        return true;

    for (size_t i = 0; i < cgiExt.size(); ++i)
    {
        if (path.find(cgiExt[i]) != std::string::npos)
            return true;
    }
    return false;
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
    _isCgi = isCgiRequest(_path);
    if (_isCgi)
        splitCgiPath(_path);
    return checkVersionAndMethod();
}

void ParserHttpRequest::splitCgiPath(const std::string &rawPath)
{
    std::string path = rawPath;

    size_t qpos = path.find('?');
    if (qpos != std::string::npos)
    {
        _queryString = path.substr(qpos + 1);
        path = path.substr(0, qpos);
    }
    else
        _queryString.clear();

const std::string arr[] = {".py", ".php", ".pl", ".cgi"};
const std::vector<std::string> cgiExt(arr, arr + sizeof(arr) / sizeof(arr[0]));    size_t extPos = std::string::npos;
    std::string foundExt;

    for (size_t i = 0; i < cgiExt.size(); ++i)
    {
        size_t pos = path.find(cgiExt[i]);
        if (pos != std::string::npos)
        {
            extPos = pos;
            foundExt = cgiExt[i];
            break;
        }
    }

    if (extPos != std::string::npos)
    {
        extPos += foundExt.length(); 
        _scriptName = path.substr(0, extPos);
        _pathInfo = path.substr(extPos);
    }
    else
    {
        _scriptName = path;
        _pathInfo.clear();
    }

    std::cout << "[CGI DETECTED] script=" << _scriptName
              << " pathInfo=" << _pathInfo
              << " query=" << _queryString << std::endl;
}

int ParserHttpRequest::parseHeaderLine()
{
    std::istringstream stream(_headerLine);
    std::string line;

    while (std::getline(stream, line, '\n'))
    {
        if (!line.empty() && (line[line.size() - 1] == '\r'))
            line = line.substr(0, line.size() - 1);

        size_t pos = line.find(':');
        if (pos == std::string::npos)
        {
            std::cout << "Invalid HTTP request: Missing ':' in header line!\n";
            return 400;
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);
        while (!value.empty() && (value[value.size() - 1] == ' ' || value[value.size() - 1] == '\t'))
            value.erase(value.size() - 1, 1);

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
                std::cout << "Duplicate key detected: [" << key << "]\n";            
                return 400;
        }
    }

    return 0;
}

void ParserHttpRequest::setBodyLine(const std::deque<std::pair<char*, ssize_t> >& bodyChunks)
{
    _bodyLine.clear();
    _bodyBuffer.clear();

    for (std::deque<std::pair<char*, ssize_t> >::const_iterator it = bodyChunks.begin();
         it != bodyChunks.end();
         ++it)
    {
        const char* data = it->first;
        ssize_t len = it->second;

        if (data && len > 0)
        {
            _bodyLine.append(data, len);
            _bodyBuffer.insert(_bodyBuffer.end(), data, data + len);
        }
    }
}

// [CHANGED] New function: releases body memory after response is built — prevents accumulation across keep-alive requests
void ParserHttpRequest::clearBody()
{
    std::string().swap(_bodyLine);
    std::vector<char>().swap(_bodyBuffer);
}

int ParserHttpRequest::parseRequest()
{
    // [CHANGED] Commented out debug logs — were printing full raw request on every parse, causing massive slowdown
    //std::cout << Colors::GREEN << "raw request initaly:  " << _rawRequest << Colors::RESET ;
    //std::cout << Colors::GREEN << "\nbodyLine intialy:  " << _bodyLine << Colors::RESET ;

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
    _rawPath = _path;
    return 0;
}

