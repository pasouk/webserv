#include "HttpResponse.hpp"

void HttpResponse::buildDelete()
{
    std::string path = _ParsedRequest.getPath();

    // N'autoriser que la suppression dans /uploads/
    //if (path.find("/uploads/") != 0)
   // {
    //    HttpResponseError(403, "Forbidden");
    //    return;
   // }

    _fullPath = path;
    if (!resourceExists(_fullPath) && !path.empty() && path[0] == '/')
        _fullPath = "uploads" + path;
    if (!resourceExists(_fullPath))
        return HttpResponseError(404, "Not Found");

    if (access(_fullPath.c_str(), W_OK) != 0)
        return HttpResponseError(403, "Forbidden");

    if (remove(_fullPath.c_str()) != 0)
        return HttpResponseError(500, "Internal Server Error");

    _body = "File deleted successfully";
    _headers["Content-Length"] = toString(_body.size());
    _headers["Content-Type"] = "text/plain";
    _headers["Connection"] = "close";
    _reason_phrase = "OK";
    _status_code = 200;
    serialize();
}