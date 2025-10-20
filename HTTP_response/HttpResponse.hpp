#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../HTTP_parser/ParserHttp.hpp"
#include "../HTTP_parser/ParserHttpRequest.hpp"
#include "HttpMultipartRequest.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <vector>

class SubPartRequest;

bool resourceExists(const std::string& path);
std::string urlDecode(const std::string& str);
bool    isFolder(std::string &path);
std::string toString(size_t n);
std::string getHeaderValue(const std::string &key, const std::map<std::string, std::string> &headers);


class   HttpResponse 
{
    private:

        const ParserHttpRequest                     _ParsedRequest;
        const int                                   _ParserExitCode;

        int                                         _status_code;
        std::string                                 _reason_phrase;
        std::map<std::string, std::string>          _headers;
        std::string                                 _body;
        std::string                                 _root;
        std::string                                 _fullPath;
        std::string                                 _formated_response;
        std::string                                 _uploads_dir;
        std::vector<HttpMethod>                     _server_methods;
    public:

        void buildFullPathGet();

        HttpResponse(ParserHttpRequest ParsedRequest, int parserExitCode);

        void HttpResponseManager();
        std::string getFormatedResponse();
        int getParserExitCode() const;
        void setRoot(std::string root);
        void setUploadDir(std::string dir);
        std::string getFullPathGet();

        void HttpResponseError(int code, const std::string& reason);
        
        std::string getContentType(const std::string &rawStr);
        void manageGetHeaders();
        void manageContentLength();
        void manageContentType();
        void managePostHeaders();

        void    buildGet();
        void    buildPost();
        void    buildDelete();

        void serialize();

        std::string extractFileName(const std::string &str) ;
        bool writeUploadedFile(std::string name)  ;

        void printElements();
        void handleMultipartPost();
                void    handleFileSubPart(const SubPartRequest &sub, const std::string &str);

        std::vector<std::string> cutMultipartPost(const std::string& rawBody, const std::string& boundary);
        void    setServerMethods(const std::vector<HttpMethod> &serverMethods);
        bool checkServerMethods(HttpMethod method);
};




#endif