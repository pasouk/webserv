#include "HttpResponse.hpp"

void HttpResponse::manageGetHeaders() 
{
    const std::map<std::string, std::string>& headers = _ParsedRequest.getHeaders();

    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end();
         ++it) 
    {
        if (it->first == "Content-Length") 
        {
            _headers["Content-Length"] = toString(_body.size());
        }
        else if (it->first == "Content-Type") 
        {
             _headers["Content-Type"] = getContentType(_fullPath);
        }
        //else 
        //{
        //    _headers[it->first] = it->second;
        //}
    }
}

void HttpResponse::buildGet()
{
    buildFullPathGet();
    if (!resourceExists(_fullPath)) 
    {
        this->HttpResponseError(404, "Not Found");
        return;
    }

    if(isFolder(_fullPath))
    {
        struct stat temp;
        std::string indexPath = _fullPath + "index.html";


        if (stat(indexPath.c_str(), &temp) == 0)
            _fullPath = indexPath;
        else 
        {
            this->HttpResponseError(403, "Forbidden");
            return;
        }
    }
    std::ifstream file(_fullPath.c_str());
    if (!file.is_open())
    {
        this->HttpResponseError(403, "Forbidden");
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    _body = buffer.str();
    file.close();
    //manageGetHeaders();
    _headers["Content-Length"] = toString(_body.size());
    _headers["Content-Type"] = getContentType(_fullPath);
    _reason_phrase = "OK";
    _status_code =  200;
    serialize();
}