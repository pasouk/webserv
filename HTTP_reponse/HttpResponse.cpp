#include "HttpResponse.hpp"

HttpResponse::HttpResponse(ParserHttpRequest ParsedRequest, int parserExitCode) : _ParsedRequest(ParsedRequest), _ParserExitCode(parserExitCode), _status_code(-1), _reason_phrase("Unprecised")
{

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

std::string HttpResponse::getFullPath()
{
    return _fullPath;
}

void HttpResponse::HttpResponseError(int code, std::string reason) 
{
    _status_code = code;
    _reason_phrase = reason;
}

void HttpResponse::buildFullPath() 
{
    if (_root.empty()) 
    {
        _fullPath = _ParsedRequest.getPath(); 
        return;
    }

    std::string fullPath = _root;

    if (!fullPath.empty() && !(_ParsedRequest.getPath().empty())) 
    {
        if (fullPath.back() == '/' && _ParsedRequest.getPath().front() == '/')
            fullPath.pop_back();
    }

    fullPath += _ParsedRequest.getPath();
    _fullPath = fullPath;
}

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

void HttpResponse::manageHeaders() 
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
        else 
        {
            _headers[it->first] = it->second;
        }
    }
}

std::string getHeaderValue(const std::string &key, const std::map<std::string, std::string> &headers)
{
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end())
        return it->second;
    return "";
}

std::string HttpResponse::extractFileName(const std::string &str) 
{
    size_t pos = str.find("filename=");

    if (pos == std::string::npos)
        return "UnknownFileName";
    pos += 9;
    
    std::string filename = str.substr(pos);

    if (!filename.empty() && filename[0] == '"') 
    {
        filename = filename.substr(1, filename.size() - 2);
    }

    return filename;
}

std::string generateUploadedFileName()
{
    time_t now = time(NULL);
    std::string tsStr;
    std::ostringstream oss;

    oss << now;
    tsStr = oss.str();
    return ("upload_" + tsStr + ".txt");

}

bool HttpReponse::writeUploadedFile(std::string name) 
{

    int fd = open(name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) 
    {
        this->HttpResponseError(500, "Internal Server Error (writing file)");
        return;
    }

    if (write(fd, _body.c_str(), _body.size()) == -1) 
    {
        close(fd);
        this->HttpResponseError(500, "Internal Server Error (writing file)");
        return;
    }

    close(fd);
}

void HttpResponse::buildPost()
{
    std::string contentLen = getHeaderValue("Content-Length", _headers);
    std::string contentVal = getHeaderValue("Content-Disposition", _headers);
    std::string fileName;

    //pour l'instant, a changer pllus tard
    setUploadDir("uploads");

    if (contentLen.empty())
    {
        HttpResponseError(400, "Bad Request")
        return;
    }

    if(!contentVal.empty())
    {
        fileName = extractFileName(contentVal);
    }
    else 
        fileName = generateUploadedFileName();
    writeUploadedFile(fileName);

}

void HttpResponse::buildGet()
{
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
    std::ifstream file(_fullPath);
    if (!file.is_open())
    {
        this->HttpResponseError(403, "Forbidden");
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    _body = buffer.str();
    file.close();
    manageHeaders();
    _reason_phrase = "OK";
    _status_code =  200;
}

void HttpResponse::serialize()
{
    std::ostringstream oss;
    oss <<  _ParsedRequest.getVersion() << " "
        << _status_code << " " 
        << _reason_phrase << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
            it != _headers.end();
            ++it)
    {
        oss << it->first << ": " << it->second << "\r\n";
    }
     // 3. Ligne vide
    oss << "\r\n";

    // 4. Body
    oss << _body;

    // Résultat final
    _formated_response = oss.str();
}

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
        return;
    }

    if (!resourceExists(_fullPath)) 
    {
        this->HttpResponseError(404, "Not Found");
        return;
    }

    
    switch (_ParsedRequest.getMethod()) {
        case GET:
            this->buildGet();
            break;
       /*case POST:
            buildPost();
            break;
        case DELETE_:
            buildDelete();
            break;*/
    }
}