#include "HttpResponse.hpp"


// constructeurs et getteurs

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

std::string HttpResponse::getFormatedResponse()
{
    return _formated_response;
}

std::string HttpResponse::getFullPathGet()
{
    return _fullPath;
}

void HttpResponse::HttpResponseError(int code, std::string reason) 
{
    _status_code = code;
    _reason_phrase = reason;
}


//Fonctions utiles a la construction de la reponse

void HttpResponse::buildFullPathGet() 
{
    if (_root.empty()) 
    {
        _fullPath = _ParsedRequest.getPath(); 
        return;
    }

    std::string fullPath = _root;

    if (!fullPath.empty() && !(_ParsedRequest.getPath().empty())) 
    {
        if (!fullPath.empty() && fullPath[fullPath.size() - 1] == '/' &&
            !_ParsedRequest.getPath().empty() && _ParsedRequest.getPath()[0] == '/')
        {
            fullPath.erase(fullPath.size() - 1);
        }
    }

    fullPath += _ParsedRequest.getPath();
    _fullPath = fullPath;
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

bool HttpResponse::writeUploadedFile(std::string name) 
{
    // Construction du chemin complet
    std::string fullName;
    if (!_uploads_dir.empty() && _uploads_dir[_uploads_dir.size()-1] != '/')
        fullName = _uploads_dir + "/" + name;
    else
        fullName = _uploads_dir + name;

    // Décodage du body
    std::string body = urlDecode(_ParsedRequest.getBodyLine());


    // Création / ouverture du fichier
    int fd = open(fullName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        HttpResponseError(500, "Internal Server Error (opening/creating file)");
        std::cout << Colors::RED << fullName << "doesnt exist\n" << Colors::RESET;
        perror("Open failed");
        
        return false;
    }

    // Écriture dans le fichier
    if (write(fd, body.c_str(), body.size()) == -1) {
        close(fd);
        HttpResponseError(500, "Internal Server Error (writing file)");
        perror("write failed");
        return false;
    }

    close(fd);

    // Mettre à jour la réponse HTTP
    _status_code = 201;           // Created
    _reason_phrase = "Created";

    return true;
}



//gestion des headers

void HttpResponse::managePostHeaders() 
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
             _headers["Content-Type"] = getContentType(_uploads_dir);
        }
    }
}

void HttpResponse::manageGetHeaders() 
{
    const std::map<std::string, std::string>& headers = _ParsedRequest.getHeaders();

    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end();
         ++it) 
    {
      //  std::cout << "\n\ndebug \n\n";
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

std::vector<std::string> HttpResponse::cutMultipartPost(const std::string& rawBody, const std::string& boundary)
{
    std::vector<std::string> sections;

    std::string sep = "--" + boundary;
    std::string endSep = sep + "--";
    size_t pos = 0;

    while (true)
    {
        size_t begin = rawBody.find(sep, pos);
        if (begin == std::string::npos)
            break;
        size_t sectionStart = begin + sep.size();

        size_t end = rawBody.find(sep, sectionStart);
        size_t endLast = rawBody.find(endSep, sectionStart);
        bool last = false;

        if (endLast != std::string::npos && (endLast < end || end == std::string::npos))
        {
            end = endLast;
            last = true;
        }

        if (end != std::string::npos && sectionStart < end)
        {
            std::string section = rawBody.substr(sectionStart, end - sectionStart);

            while (!section.empty() && (section[0] == '\r' || section[0] == '\n'))
                section.erase(0, 1);
            while (!section.empty() && (section[section.size() - 1] == '\r' || section[section.size() - 1] == '\n'))
                section.erase(section.size() - 1);

            sections.push_back(section);
        }

        if (last)
            break;

        pos = end;
    }

    return sections;
}



    //fonctions principales par méthode 
    void HttpResponse::buildPost()
{
    std::string ctype = getHeaderValue("Content-Type", _ParsedRequest.getHeaders());
    if (ctype.find("multipart/form-data") != std::string::npos) 
    {
        handleMultipartPost();
        return;
    }   
    std::string contentLen = getHeaderValue("Content-Length", _ParsedRequest.getHeaders());
    std::string contentVal = getHeaderValue("Content-Disposition", _ParsedRequest.getHeaders());
    std::string fileName;

    //std::cout << Colors::RED << "contentlen: " << contentLen << std::endl << "contentval:" <<  contentVal << Colors::RESET << std::endl;

    //pour l'instant, a changer pllus tard
    setUploadDir("uploads/");

    if (contentLen.empty())
    {
        HttpResponseError(400, "Bad Request");
        return;
    }

    if(!contentVal.empty())
    {
        fileName = extractFileName(contentVal);
        
    }
    else 
        fileName = generateUploadedFileName();
   // std::cout << Colors::RED << fileName << Colors::RESET;

    if(!writeUploadedFile(fileName))
        return;
    _status_code = 201; 
    _reason_phrase = "Created";
    _body = "File uploaded successfully";
    _headers["Content-Length"] = toString(_body.size());
    _headers["Content-Type"] = "text/plain";
    serialize();

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



void HttpResponse::handleFileSubPart(const SubPartRequest &sub, const std::string &str)
{
    // Récupération du nom de fichier
    size_t start = str.find("filename=\"");
    if (start == std::string::npos)
    {
        std::cout << "No filename found in Content-Disposition.\n";
        return;
    }
    start += 10;
    size_t end = str.find("\"", start);
    std::string filename = str.substr(start, end - start);

    _uploads_dir = "uploads";
    // Préparation du chemin complet
    std::string fullName = _uploads_dir + "/" + filename;

    // Ouverture du fichier
    int fd = open(fullName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        std::cout << "Error opening file " << filename << " (check rights / path)\n";
        perror("open");
        return;
    }

    // Nettoyage du body (supprime \r\n final)
    std::string body = sub.getBodyLine();
    if (body.size() >= 2 && body.substr(body.size() - 2) == "\r\n")
        body = body.substr(0, body.size() - 2);

    // Écriture dans le fichier
    ssize_t written = write(fd, body.c_str(), body.size());
    if (written == -1)
        perror("write");

    close(fd);
    _status_code = 201; 
    _reason_phrase = "Created";
    _body = "File uploaded successfully";
    _headers["Content-Length"] = toString(_body.size());
    _headers["Content-Type"] = "text/plain";
    serialize();
}

void HttpResponse::handleMultipartPost()
{
    std::string contentType = getHeaderValue("Content-Type", _ParsedRequest.getHeaders());

    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
        return;

    std::string boundary = contentType.substr(pos + 9);
    std::vector<std::string> devidedBody = cutMultipartPost(_ParsedRequest.getBodyLine(), boundary);

    for(long unsigned int i = 0; i < devidedBody.size(); i++)
    {
        SubPartRequest sub(devidedBody[i]);
        sub.devideRequest();
        sub.parseHeaderLine();  
        std::string cd = getHeaderValue("Content-Disposition", sub.getHeaders());
        if (cd.find("filename=") != std::string::npos)
        {
            handleFileSubPart(sub, cd);
        }
        sub.printParsedData();
    }
        
    //std::cout << _ParsedRequest.getBodyLine();
}
//fonction principale

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
            //buildDelete();
            break;
        case UNKNOWN:
            //buildDelete();
            break;
    }
}

//mise en forme de la reponse
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



//fonctions de debug

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