#include "HttpResponse.hpp"

void HttpResponse::printElements()
{
    std::cout << Colors::BLUE << "Status code : " << Colors::RESET << _status_code << std::endl;
    std::cout << Colors::BLUE << "Reason : " << Colors::RESET << _reason_phrase << std::endl;
    std::cout << Colors::BLUE << "body : " << Colors::RESET << _body << std::endl;
    std::cout << Colors::BLUE << "Root : " << Colors::RESET << _root<< std::endl;
    std::cout << Colors::BLUE << "Full path : " << Colors::RESET << _fullPath << std::endl;
    std::cout << Colors::BLUE << "Headers : " << Colors::RESET << std::endl; 
    const std::map<std::string, std::string>& headers = _headers;
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end();
         ++it) 
    {
        std::cout << it->first << " : " << it->second << std::endl;
    }
    std::cout << Colors::BLUE << "Formated Response: " << Colors::RESET << _formated_response << std::endl;
} 

void HttpResponse::serialize()
{
    std::ostringstream oss;

    time_t _now = time(0);
    char _dateBuf[80];
    struct tm *_tm = gmtime(&_now);
    strftime(_dateBuf, sizeof(_dateBuf), "%a, %d %b %Y %H:%M:%S GMT", _tm);
    _headers["Date"] = _dateBuf;
    _headers["Server"] = "webserv/1.0";

    oss <<  _ParsedRequest.getVersion() << " "
        << _status_code << " "
        << _reason_phrase << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
            it != _headers.end();
            ++it)
    {
        oss << it->first << ": " << it->second << "\r\n";
    }
    oss << "\r\n";
    oss << _body;
    _formated_response = oss.str();
}

bool HttpResponse::checkServerMethods(HttpMethod method)
{
    if (_server_methods.empty())
        return true;

    for (std::vector<HttpMethod>::const_iterator it = _server_methods.begin();
         it != _server_methods.end(); ++it)
    {
        if (*it == method)
            return true;
    }
    return false;
}

void HttpResponse::buildFullPathGet()
{
    //std::cout << "[DEBUG buildFullPathGet] START - root='" << _root << "' path='" << _ParsedRequest.getPath() << "'" << std::endl;

    if (_root.empty())
    {
        _fullPath = _ParsedRequest.getPath();
        //std::cout << "[DEBUG buildFullPathGet] Empty root, using path directly: '" << _fullPath << "'" << std::endl;
        return;
    }

    std::string fullPath = _root;

    if (!fullPath.empty() && !(_ParsedRequest.getPath().empty()))
    {
        if (!fullPath.empty() && fullPath[fullPath.size() - 1] == '/' &&
            !_ParsedRequest.getPath().empty() && _ParsedRequest.getPath()[0] == '/')
        {
            //std::cout << "[DEBUG buildFullPathGet] Removing duplicate '/' from root" << std::endl;
            fullPath.erase(fullPath.size() - 1);
        }
    }

    fullPath += _ParsedRequest.getPath();
    _fullPath = fullPath;
    //std::cout << "[DEBUG buildFullPathGet] Final fullPath='" << _fullPath << "'" << std::endl;
    //{
    //    std::ostringstream oss;
    //    oss << " -> root:'" << _root << "' path:'" << _ParsedRequest.getPath() << "' full:'" << _fullPath << "'";
    //    logOutMessage(oss);
    //}
}

void HttpResponse::HttpResponseError(int code, const std::string& reason)
{
    _status_code = code;
    _reason_phrase = reason;

    std::string errorPagePath;
    std::map<int, std::string>::const_iterator locErr = _matchedLocation.error_pages.find(code);
    if (locErr != _matchedLocation.error_pages.end())
        errorPagePath = locErr->second;
    else
    {
        std::map<int, std::string>::const_iterator servErr = _serverErrorPages.find(code);
        if (servErr != _serverErrorPages.end())
            errorPagePath = servErr->second;
    }

    if (!errorPagePath.empty())
    {
        std::string finalPath = errorPagePath;
        if (!resourceExists(finalPath) && !finalPath.empty() && finalPath[0] == '/' && !_root.empty())
        {
            std::string joined = _root;
            if (!joined.empty() && joined[joined.size() - 1] == '/' && finalPath[0] == '/')
                joined.erase(joined.size() - 1);
            joined += finalPath;
            if (resourceExists(joined))
                finalPath = joined;
        }
        if (resourceExists(finalPath))
        {
            std::ifstream file(finalPath.c_str());
            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                _body = buffer.str();
            }
        }
    }

    if (_body.empty())
    {
        std::ostringstream oss;
        oss << "<html><head><title>" << code << " " << reason << "</title></head>\n"
            << "<body><h1>" << code << " " << reason << "</h1>\n"
            << "<p>The server returned an error.</p>\n"
            << "</body></html>";
        _body = oss.str();
    }

    _headers["Content-Type"] = "text/html";
    _headers["Content-Length"] = toString(_body.size());

    _headers["Connection"] = "close";

    if (_status_code == 405)
    {
        _headers["Allow"] = "GET, POST, DELETE";
    }

    if (_status_code == 403)
    {
        std::ostringstream oss;
        oss << "HttpResponseError 403 for path: '" << _ParsedRequest.getPath() << "' raw: '" << _ParsedRequest.getRawPath() << "'";
        logOutMessage(oss);
    }

    serialize();
}

std::string HttpResponse::getContentType(const std::string &rawStr)
{
    size_t dotPos = rawStr.find_last_of('.');
    if (dotPos == std::string::npos)
        return "application/octet-stream";

    std::string extension = rawStr.substr(dotPos + 1);

    for (size_t i = 0; i < extension.size(); ++i)
        extension[i] = tolower(extension[i]);

    if (extension == "html") return "text/html";
    else if (extension == "css") return "text/css";
    else if (extension == "txt") return "text/plain";
    else if (extension == "xml") return "text/xml";
    else if (extension == "json") return "application/json";
    else if (extension == "xhtml") return "application/xhtml+xml";
    else if (extension == "png") return "image/png";
    else if (extension == "pdf") return "application/pdf";
    else return "application/octet-stream";
}


s_location HttpResponse::matchLocation()
{
    s_location best;
    size_t bestMatchLen = 0;
    const std::string& rawPath = _ParsedRequest.getRawPath();

    for (size_t i = 0; i < _locations.size(); ++i)
    {
        const s_location& loc = _locations[i];

        //std::cout << Colors::GREEN
        //          << "rawPath: " << rawPath
        //          << "\nconcatOrReplace: " << loc.concatOrReplace
        //          << "\nby: " << loc.by
        //          << Colors::RESET << std::endl << std::endl;

        if (rawPath.compare(0, loc.concatOrReplace.size(), loc.concatOrReplace) == 0)
        {
            if (loc.concatOrReplace.size() > bestMatchLen)
            {
                best = loc;
                bestMatchLen = loc.concatOrReplace.size();
            }
        }
    }

    _matchedLocation = best;
    return best;
}

void HttpResponse::applyLocationConfig(const s_location& loc)
{
    // max_body_size
    if (!loc.max_body_size.empty() && loc.max_body_size != "not define")
    {
        std::stringstream ss(loc.max_body_size);
        ss >> _LocationMaxBodySize;
    }
    else
    {
        _LocationMaxBodySize = _serverMaxBodySize;
    }

    _LocationMethodsAllowed = loc.httpMethodsAllowed;

    // index file
    if (!loc.index.empty())
        _locationIndex = loc.index;
    else
        _locationIndex = "index.html";

    _matchedLocation = loc;
}