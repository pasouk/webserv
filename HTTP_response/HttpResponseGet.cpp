#include "HttpResponse.hpp"

void HttpResponse::buildHead()
{
    buildGet();
    _status_code = 405;
    _body.clear();
    _headers["Content-Length"] = "0";
    serialize();
}

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
    //std::cout << "DEBUG buildGet: fullPath = '" << _fullPath << "'" << std::endl;

    // Check if we need to redirect to add trailing slash for directories
    std::string requestedPath = _ParsedRequest.getPath();
    if (!requestedPath.empty() && requestedPath[requestedPath.size() - 1] != '/')
    {
        // Check if there's a location with trailing slash
        std::string pathWithSlash = requestedPath + "/";
        for (size_t i = 0; i < _locations.size(); ++i)
        {
            if (_locations[i].concatOrReplace == pathWithSlash)
            {
                //std::cout << "[DEBUG buildGet] Found location '" << pathWithSlash << "', redirecting with 301" << std::endl;
                _status_code = 301;
                _reason_phrase = "Moved Permanently";
                _headers["Location"] = pathWithSlash;
                _headers["Content-Length"] = "0";
                serialize();
                return;
            }
        }
    }
    
    //std::cout << "[DEBUG buildGet] Checking if resource exists at: '" << _fullPath << "'" << std::endl;

    if (!resourceExists(_fullPath))
    {
        //std::cout << "[DEBUG buildGet] Resource NOT found at '" << _fullPath << "'" << std::endl;
        this->HttpResponseError(404, "Not Found here");
        return;
    }
    //std::cout << "[DEBUG buildGet] Resource FOUND at '" << _fullPath << "'" << std::endl;

    if(isFolder(_fullPath))
    {
        struct stat temp;
        std::string indexPath = _fullPath;
        // Add trailing slash if not present
        if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
            indexPath += "/";
        indexPath += _locationIndex;
        //std::cout << "[DEBUG buildGet] Path is a folder, looking for index at: '" << indexPath << "'" << std::endl;

        if (stat(indexPath.c_str(), &temp) == 0)
        {
            //std::cout << "[DEBUG buildGet] Found index '" << _locationIndex << "' at '" << indexPath << "'" << std::endl;
            _fullPath = indexPath;
        }
        else
        {
            //std::cout << "[DEBUG buildGet] Index '" << _locationIndex << "' NOT found at '" << indexPath << "'" << std::endl;
            // [CHANGED] If autoindex is on and no index file exists, generate a directory listing instead of 404
            if (_matchedLocation.autoindex)
            {
                buildAutoIndex(_fullPath, _matchedLocation.concatOrReplace);
                return;
            }
            // Directory exists but configured index is missing -> behave as not found
            this->HttpResponseError(404, "Not Found here");
            return;
        }
    }
    std::ifstream file(_fullPath.c_str());
    if (!file.is_open())
    {
        //std::cout << "[DEBUG buildGet] Cannot open file: '" << _fullPath << "'" << std::endl;
        this->HttpResponseError(403, "Forbidden");
        return;
    }
    //std::cout << "[DEBUG buildGet] Successfully opened file: '" << _fullPath << "'" << std::endl;
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

// [CHANGED] New function: generates an HTML page listing directory contents (autoindex directive)
void HttpResponse::buildAutoIndex(const std::string& dirPath, const std::string& urlPath)
{
    DIR* dir = opendir(dirPath.c_str());
    if (!dir)
    {
        HttpResponseError(403, "Forbidden");
        return;
    }

    std::string urlBase = urlPath;
    if (!urlBase.empty() && urlBase[urlBase.size() - 1] != '/')
        urlBase += "/";

    std::ostringstream html;
    html << "<!DOCTYPE html><html><head><title>Index of " << urlPath << "</title>"
         << "<style>"
         << "body{font-family:monospace;max-width:800px;margin:40px auto;padding:0 20px}"
         << "h1{border-bottom:1px solid #ccc;padding-bottom:8px}"
         << "a{display:block;padding:2px 0;text-decoration:none;color:#00e}"
         << "a:hover{text-decoration:underline}"
         << "</style></head>\n"
         << "<body><h1>Index of " << urlPath << "</h1><pre>\n";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == ".")
            continue;
        html << "<a href=\"" << urlBase << name << "\">" << name << "</a>\n";
    }
    closedir(dir);

    html << "</pre><hr></body></html>";
    _body = html.str();
    _headers["Content-Type"] = "text/html";
    _headers["Content-Length"] = toString(_body.size());
    _reason_phrase = "OK";
    _status_code = 200;
    serialize();
}