#include "ParserHttp.hpp"

int main()
{
    std::string rawRequest = "POST / HTTP/1.1\r\n \
        Host: localhost:8080\r\n \
        User-Agent: curl/7.81.0\r\n \
        Accept: */*\r\n \
        Content-Length: 7\r\n \
        Content-Type: application/x-www-form-urlencoded\r\n\r\n name=42";


    ParserHttpRequest request1(rawRequest);
    int ret = request1.parseRequest();
    if(ret)
    {
        std::cout << Colors::RED << "Parsing exit code : " << ret << Colors::RESET << std::endl;
        return 0;
    }
    else 
        std::cout << Colors::GREEN << "Parsing exit code : " << ret << Colors::RESET << std::endl;

    request1.printParsedData();
    
    //request1.debugParsingRequest();
    
    
}