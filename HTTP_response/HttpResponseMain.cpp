#include "HttpResponse.hpp"

HttpResponse::HttpResponse(ParserHttpRequest ParsedRequest, int parserExitCode) : _ParsedRequest(ParsedRequest), _ParserExitCode(parserExitCode), _status_code(-1), _reason_phrase("Unprecised")
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

std::string HttpResponse::getFormatedResponse()
{
    return _formated_response;
}

std::string HttpResponse::getFullPathGet()
{
    return _fullPath;
}

//Main function for building request
void HttpResponse::HttpResponseManager() 
{
    if (_ParserExitCode != 0) 
    {
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
    if(checkServerMethods(_ParsedRequest.getMethod()) == false)
    {
        this->HttpResponseError(405, "Method Not Allowed by server configuration");
        _headers["Allow"] = "GET, POST, DELETE";
        _headers["Content-Length"] = "0";
        serialize();
        return;
    }
    
    switch (_ParsedRequest.getMethod()) {
        case GET:
            this->buildGet();
            break;
       case POST:
            this->buildPost();
            break;
        case DELETE_:
            buildDelete();
            break;
        case UNKNOWN:
            break;
    }
}

