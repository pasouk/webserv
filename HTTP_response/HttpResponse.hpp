#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../HTTP_parser/ParserHttp.hpp"
#include "../HTTP_parser/ParserHttpRequest.hpp"
#include "HttpMultipartRequest.hpp"
#include <sys/stat.h>
#include <dirent.h> 
#include <fstream>
#include <sstream>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <vector>
#include "../headers/Webserv.hpp"

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
        std::vector<s_location>                     _locations;
        s_location                                  _matchedLocation;
        size_t                                      _LocationMaxBodySize;
        std::vector<HttpMethod>                     _LocationMethodsAllowed;
        std::string                                 _locationIndex;
        size_t _serverMaxBodySize;
        std::map<int, std::string>                  _serverErrorPages;

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
        void            setLocations(const std::vector<s_location> &locations);
        void            setMatchedLocation(const s_location& loc);
        void            setServerErrorPages(const std::map<int, std::string>& errorPages);

        //Main
        void HttpResponseManager();

        //GET Method
        void    buildGet();
        void    buildAutoIndex(const std::string& dirPath, const std::string& urlPath);
        void manageGetHeaders();
        void buildHead();

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
        void applyLocationConfig(const s_location& best);
        s_location matchLocation();
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