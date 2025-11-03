# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fbuyl <fbuyl@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/09 13:44:18 by fabricebuyl       #+#    #+#              #
#    Updated: 2025/11/03 12:04:14 by fbuyl            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = webserv
RELATIVE = true

SOURCES = QueryListener.cpp \
	ConfigParser.cpp \
	Node.cpp \
	NodeDirective.cpp \
	NodeBlock.cpp \
	Directives.cpp \
	Webserv.cpp \
	Webserv2.cpp \
	Webserv3.cpp \
	main.cpp \
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
	logtime.cpp 
	

	
OBJECTS = $(SOURCES:.cpp=.o)

all: $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -DRELATIVE=$(RELATIVE) -I./headers -I./HTTP_parser -I./HTTP_response -I./CGI -c $< -o $@

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I./headers -I./HTTP_parser -I./HTTP_response -I./CGI  -o $(NAME) $(OBJECTS)

clean:
	rm -f HTTP_parser/*.o
	rm -f HTTP_response/*.o
	rm -f CGI/*.o
	rm -f *.o

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re
