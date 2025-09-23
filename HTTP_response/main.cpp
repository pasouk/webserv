
#include "HttpResponse.hpp"

int main()
{
   std::string rawRequest = "GET /index.html HTTP/1.1\r\n \
                                Host: localhost:8080\r\n \
                                User-Agent: curl/7.81.0\r\n \
                                Accept: */*\r\n\r\n  ";

    std::string rawRequest2 = "GET / HTTP/1.1\r\n \
                                Host: localhost:8080\r\n \
                                User-Agent: curl/7.81.0\r\n \
                                Accept: */*\r\n\r\n";

    std::string rawRequest3 = "POST /upload HTTP/1.1\r\n \
                                    Host: localhost:8080\r\n \
                                    User-Agent: curl/7.81.0\r\n \
                                    Content-Type: text/plain\r\n \
                                    Content-Length: 11\r\n\r\n \
                                    hello world";

    std::string rawRequest4 = "DELETE /old.txt HTTP/1.1\r\n \
                                Host: localhost:8080\r\n \
                                User-Agent: curl/7.81.0\r\n\r\n";




  //  request1.debugParsingRequest();


    //a changer plus tard 
    std::string root = "/home/pasouk/webserv";
    


   // response1.buildFullPathGet();
   // std::cout << "parsing exit code : " << response1.getParserExitCode() << std::endl;
    
//    response1.serialize();
    ParserHttpRequest request1(rawRequest);
    int ret_parsing = request1.parseRequest();
    HttpResponse response1(request1, ret_parsing);
    response1.setRoot(root);
    //request1.printParsedData();
    response1.HttpResponseManager();
   // request1.debugParsingRequest();
    //response1.printElements();
    
    std::cout << Colors::BLUE <<  "response 1: " << Colors::RESET << std::endl << response1.getFormatedResponse() << std::endl << std::endl;

    ParserHttpRequest request2(rawRequest2);
    int ret_parsing2 = request2.parseRequest();
    HttpResponse response2(request2, ret_parsing2);
    response2.setRoot(root);
   // request2.printParsedData();
    response2.HttpResponseManager();
    //response2.printElements();
    std::cout << Colors::BLUE <<  "response 2: " << Colors::RESET << std::endl << response2.getFormatedResponse() << std::endl << std::endl;

    
    ParserHttpRequest request3(rawRequest3);
    int ret_parsing3 = request3.parseRequest();
    HttpResponse response3(request3, ret_parsing3);
    response3.setRoot(root);
    //request3.printParsedData();
    response3.HttpResponseManager();
    //response3.printElements();
    std::cout << Colors::BLUE <<  "response 3: " << Colors::RESET << std::endl << response3.getFormatedResponse() << std::endl << std::endl;


    ParserHttpRequest request4(rawRequest4);
    int ret_parsing4 = request4.parseRequest();
    HttpResponse response4(request4, ret_parsing4);
    response4.setRoot(root);
   // request4.printParsedData();
    response4.HttpResponseManager();
    //response4.printElements();
    std::cout << Colors::BLUE <<  "response 4: " << Colors::RESET << std::endl << response4.getFormatedResponse() << std::endl << std::endl;


}