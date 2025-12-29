# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fabrice <fabrice@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/09 13:44:18 by fabricebuyl       #+#    #+#              #
#    Updated: 2025/12/29 14:32:50 by fabrice          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = webserv

SOURCES = QueryListener.cpp \
	ConfigParser.cpp \
	Node.cpp \
	NodeDirective.cpp \
	NodeBlock.cpp \
	Directives.cpp \
	Webserv.cpp \
	Webserv2.cpp \
	Webserv3.cpp \
	CGI/CGI.cpp \
	utils.cpp \
	TransferEncoding/Chunked.cpp \
	TransferEncoding/TransferEncoding.cpp \
	TransferEncoding/ContentLength.cpp \
	HTTP_parser/ParserHttpRequest.cpp\
	HTTP_parser/ParsingUtils.cpp \
	HTTP_parser/ParsingChecks.cpp \
	HTTP_parser/Utils.cpp \
	HTTP_response/HttpResponseUtils.cpp \
	HTTP_response/HttpResponseMain.cpp \
	HTTP_response/HttpMultipartRequest.cpp \
	HTTP_response/HttpResponseDelete.cpp \
	HTTP_response/HttpResponseGet.cpp \
	HTTP_response/HttpResponsePost.cpp \
	HTTP_response/Utils.cpp \
	main.cpp
		
OBJECTS = $(SOURCES:.cpp=.o)

all: $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I./headers -I./HTTP_parser -I./HTTP_response \
	-I./CGI -I./TransferEncoding -c $< -o $@

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I./headers -I./HTTP_parser -I./HTTP_response \
	-I./CGI -I./TransferEncoding -o $(NAME) $(OBJECTS)

clean:
	rm -f HTTP_parser/*.o
	rm -f HTTP_response/*.o
	rm -f CGI/*.o
	rm -f TransferEncoding/*.o
	rm -f *.o

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re
