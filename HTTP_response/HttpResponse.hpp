#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../HTTP_parser/ParserHttp.hpp"
#include "../HTTP_parser/ParserHttpRequest.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>

bool resourceExists(const std::string& path);
bool    isFolder(std::string &path);
std::string toString(size_t n);

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

    public:

        void buildFullPathGet();

        HttpResponse(ParserHttpRequest ParsedRequest, int parserExitCode);

        void HttpResponseManager();
        std::string getFormatedResponse();
        int getParserExitCode() const;
        void setRoot(std::string root);
        void setUploadDir(std::string dir);
        std::string getFullPathGet();

        void HttpResponseError(int code, std::string reason);
        
        std::string getContentType(const std::string &rawStr);
        void manageGetHeaders();
        void manageContentLength();
        void manageContentType();
        void managePostHeaders();

        void    buildGet();
        void    buildPost();
        //void    buildDelete();

        void serialize();

        std::string extractFileName(const std::string &str) ;
        bool writeUploadedFile(std::string name)  ;

        void printElements();

};




#endif