#ifndef HTTPMULTIPARTREQUEST_HPP
#define HTTPMULTIPARTREQUEST_HPP

#include "HttpResponse.hpp"

class SubPartRequest
{
    private:
        std::string     _rawRequest;
        std::string     _headerLine;
        std::string     _bodyLine;
        std::deque<std::pair<char*, ssize_t> > _bodyBuffer;
        std::map<std::string, std::string>   _headers;

    public:
        SubPartRequest(std::string rawRequest);

        std::string getHeaderLine();
        std::string getBodyLine();
        std::string getBodyLine() const;

        const std::map<std::string, std::string>& getHeaders() const;
        

        void        devideRequest();
        int        parseHeaderLine();

        bool    isSeveralLines();
        bool    isBodySeparator();
        int     basicChecks();
        //int     debugParsingRequest();
        //void    sanitize();
        void    printParsedData();
        const std::deque<std::pair<char*, ssize_t> >& getBodyBuffer() const ;

};

#endif