
#include "HttpResponse.hpp"

int main()
{
   std::string rawRequest = "GET / HTTP/1.1\r\n \
        Host: localhost:8080\r\n \
        User-Agent: curl/7.81.0\r\n \
        Accept: */*\r\n \
        Content-Length: 7\r\n \
        Content-Type: application/x-www-form-urlencoded\r\n\r\n name=42";



    ParserHttpRequest request1(rawRequest);
    
    int ret_parsing = request1.parseRequest();
    HttpResponse response1(request1, ret_parsing);
    
    //a changer plus tard 
    std::string root = "/home/pasouk/webserv";
    response1.setRoot(root);


    response1.buildFullPath();
    std::cout << "parsing exit code : " << response1.getParserExitCode() << std::endl;
    response1.HttpResponseManager();
    response1.serialize();
    response1.printElements();


}