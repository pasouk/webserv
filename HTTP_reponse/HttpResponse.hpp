#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../HTTP_parser/ParserHttp.hpp"
#include "../HTTP_parser/ParserHttpRequest.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>

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

    public:

        void buildFullPath();

        HttpResponse(ParserHttpRequest ParsedRequest, int parserExitCode);

        void HttpResponseManager();

        int getParserExitCode() const;
        void setRoot(std::string root);
        std::string getFullPath();

        void HttpResponseError(int code, std::string reason);
        
        std::string getContentType(const std::string &rawStr);
        void manageHeaders();
        void manageContentLength();
        void manageContentType();

        void    buildGet();
        //void    buildPost();
        //void    buildDelete();


        void printElements();

};




#endif