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

        //constructor
        HttpResponse(ParserHttpRequest ParsedRequest, int parserExitCode);

        //getters and setters
        std::string     getFormatedResponse();
        int             getParserExitCode() const;
        void            setRoot(std::string root);
        void            setUploadDir(std::string dir);
        std::string     getFullPathGet();
        std::string     getContentType(const std::string &rawStr);
        void            setServerMethods(const std::vector<HttpMethod> &serverMethods);

        //Main
        void HttpResponseManager();

        //GET Method
        void    buildGet();
        void manageGetHeaders();

        //POST Method
        void                        handleMultipartPost();
        void                        handleFileSubPart(const SubPartRequest &sub, const std::string &str);
        std::vector<std::string>    cutMultipartPost(const std::string& rawBody, const std::string& boundary);
        void                        managePostHeaders();
        void                        buildPost();
        bool                        writeUploadedFile(std::string name);
        std::string                 extractFileName(const std::string &str) ;
        


        //DELETE Method
        void buildDelete();

        //Class utils
        void printElements();
        void serialize();
        void buildFullPathGet();
        bool checkServerMethods(HttpMethod method);
        void HttpResponseError(int code, const std::string& reason);
};

//Generic utils
bool resourceExists(const std::string& path);
std::string urlDecode(const std::string& str);
bool    isFolder(std::string &path);
std::string toString(size_t n);
std::string getHeaderValue(const std::string &key, const std::map<std::string, std::string> &headers);
std::string generateUploadedFileName();
void managePostHeaders();

#endif