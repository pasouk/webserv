#include "HttpResponse.hpp"

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

bool HttpResponse::writeUploadedFile(std::string name) 
{
    std::string fullName;
    if (!_uploads_dir.empty() && _uploads_dir[_uploads_dir.size()-1] != '/')
        fullName = _uploads_dir + "/" + name;
    else
        fullName = _uploads_dir + name;

    std::string body = urlDecode(_ParsedRequest.getBodyLine());


    std::ofstream ofs(fullName.c_str(), std::ios::out | std::ios::trunc);
    if (!ofs.is_open())
    {
        HttpResponseError(500, "Internal Server Error (opening/creating file)");
        return false;
    }
    ofs.write(body.c_str(), body.size());
    if (!ofs)
    {
        HttpResponseError(500, "Internal Server Error (writing file)");
        return false;
    }
    ofs.close();

    _status_code = 201;
    _reason_phrase = "Created";
    return true;
}

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

void HttpResponse::handleFileSubPart(const SubPartRequest &sub, const std::string &cd)
{
    size_t start = cd.find("filename=\"");
    if (start == std::string::npos)
    {
        std::cout << "No filename found in Content-Disposition.\n";
        return;
    }
    start += 10;
    size_t end = cd.find("\"", start);
    std::string filename = cd.substr(start, end - start);

    _uploads_dir = "uploads";
    std::string fullName = _uploads_dir + "/" + filename;
    std::ofstream ofs(fullName.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
    {
        return;
    }

    // Détection du Content-Type pour décider du mode d’écriture
    std::string contentType = getHeaderValue("Content-Type", sub.getHeaders());
    if (!contentType.empty() && contentType.find("text/") == 0)
    {
        // si texte
        std::string body = sub.getBodyLine();
        if (body.size() >= 2 && body.substr(body.size() - 2) == "\r\n")
            body = body.substr(0, body.size() - 2);
        ofs.write(body.c_str(), body.size());
        if (!ofs)
            return ;
    }
    else
    {
        // si bin
        const std::deque<std::pair<char*, ssize_t> >& chunks = sub.getBodyBuffer();
        for (std::deque<std::pair<char*, ssize_t> >::const_iterator it = chunks.begin();
             it != chunks.end(); ++it)
        {
            if (it->first && it->second > 0)
            {
                ofs.write(it->first, it->second);
                if (!ofs)
                    return ;
            }
        }
    }
    ofs.close();
    _status_code = 201;
    _reason_phrase = "Created";
    _body = "File uploaded successfully";
    _headers["Content-Length"] = toString(_body.size());
    _headers["Content-Type"] = "text/plain";
    serialize();
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
    }
}

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

    setUploadDir("uploads/");

    size_t declaredLen = 0;
    if (!contentLen.empty())
    {
        std::stringstream ssLen(contentLen);
        ssLen >> declaredLen;
    }
    else
    {
        declaredLen = _ParsedRequest.getBodyLine().size();
    }

    if (declaredLen > _LocationMaxBodySize)
    {
        HttpResponseError(413, "Request Entity Too Large");
        return;
    }

    if (declaredLen == 0)
    {
        _status_code = 200;
        _reason_phrase = "OK";
        _body = "File uploaded successfully";
        _headers["Content-Length"] = toString(_body.size());
        _headers["Content-Type"] = "text/plain";
        serialize();
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