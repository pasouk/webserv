#include "HttpResponse.hpp"

HttpResponse::HttpResponse(ParserHttpRequest ParsedRequest, int parserExitCode)
  : _ParsedRequest(ParsedRequest)
  , _ParserExitCode(parserExitCode)
  , _status_code(-1)
  , _reason_phrase("Unprecised")
  , _LocationMaxBodySize(8192)
  , _locationIndex("index.html")
  , _serverMaxBodySize(8192)
{
}

void    HttpResponse::setServerMethods(const std::vector<HttpMethod> &serverMethods)
{
    _server_methods = serverMethods;
}


int HttpResponse::getParserExitCode() const
{
    return _ParserExitCode;
}

void HttpResponse::setRoot(std::string root)
{
    _root = root;
}

void HttpResponse::setUploadDir(std::string dir)
{
    _uploads_dir = dir;
}

void HttpResponse::setLocations(const std::vector<s_location> &locations)
{
    _locations = locations;
}

void HttpResponse::setMatchedLocation(const s_location& loc)
{
    _matchedLocation = loc;
}

void HttpResponse::setServerErrorPages(const std::map<int, std::string>& errorPages)
{
    _serverErrorPages = errorPages;
}

std::string HttpResponse::getFormatedResponse()
{
    return _formated_response;
}

std::string HttpResponse::getFullPathGet()
{
    return _fullPath;
}

void HttpResponse::HttpResponseManager() 
{
    if (_ParserExitCode != 0)
    {
        if (_ParserExitCode == 413)
        {
            this->HttpResponseError(413, "Request Entity Too Large");
            return;
        }
        this->HttpResponseError(400, "Bad Request");
        return;
    }

    if (_ParsedRequest.getMethod() == UNKNOWN) 
    {
        this->HttpResponseError(405, "Method Not Allowed");
        _headers["Allow"] = "GET, POST, DELETE";
        _headers["Content-Length"] = "0";
        serialize();
        return;
    }
    
    applyLocationConfig(_matchedLocation);
    
    //std::cout << "[DEBUG HttpResponseManager] Using matched location with " << _matchedLocation.httpMethodsAllowed.size() << " allowed methods" << std::endl;
    
    if (!_matchedLocation.httpMethodsAllowed.empty())
    {
        bool methodAllowed = false;
        for (size_t i = 0; i < _matchedLocation.httpMethodsAllowed.size(); ++i)
        {
            if (_matchedLocation.httpMethodsAllowed[i] == _ParsedRequest.getMethod())
            {
                methodAllowed = true;
                break;
            }
        }
        
        if (!methodAllowed)
        {
            //std::cout << "[DEBUG HttpResponseManager] Method " << _ParsedRequest.getMethod()
            //          << " NOT allowed for location '" << _matchedLocation.concatOrReplace << "'" << std::endl;
            this->HttpResponseError(405, "Method Not Allowed");
            _headers["Allow"] = "GET, POST, DELETE";
            _headers["Content-Length"] = "0";
            serialize();
            return;
        }
    }
    else if(!checkServerMethods(_ParsedRequest.getMethod()))
    {
        //std::cout << "[DEBUG HttpResponseManager] Method " << _ParsedRequest.getMethod()
        //          << " NOT allowed by server configuration" << std::endl;
        this->HttpResponseError(405, "Method Not Allowed by server configuration");
        _headers["Allow"] = "GET, POST, DELETE";
        _headers["Content-Length"] = "0";
        serialize();
        return;
    }
    
    //std::cout << "[DEBUG HttpResponseManager] Method " << _ParsedRequest.getMethod() << " ALLOWED" << std::endl;
    
    switch (_ParsedRequest.getMethod()) 
    {
        case HEAD:
            this->buildHead();
            break;
        case GET:
            this->buildGet();
            break;
        case POST:
            this->buildPost();
            break;
        case DELETE_:
            this->buildDelete();
            break;
        case UNKNOWN:
            break;
    }
}

