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

void HttpResponse::HttpResponseError(int code, const std::string& reason)
{
    _status_code = code;
    _reason_phrase = reason;

    // Si aucun body n'a été défini, construire un body d'erreur simple
    if (_body.empty())
    {
        std::ostringstream oss;
        oss << "<html><head><title>" << code << " " << reason << "</title></head>\n"
            << "<body><h1>" << code << " " << reason << "</h1>\n"
            << "<p>The server returned an error.</p>\n"
            << "</body></html>";
        _body = oss.str();
    }

    // Headers minimaux nécessaires
    _headers["Content-Type"] = "text/html";
    _headers["Content-Length"] = toString(_body.size());

    // Par défaut fermer la connexion après l'erreur (sécurise le client)
    _headers["Connection"] = "close";

    // Si c'est un 405, indiquer les méthodes autorisées
    if (_status_code == 405)
    {
        // adapte selon ta config réelle
        _headers["Allow"] = "GET, POST, DELETE";
    }

    // Construire la réponse formatée maintenant (optionnel)
    // serialize() doit assembler la première ligne + headers + \r\n + body
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