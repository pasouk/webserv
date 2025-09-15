# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/09 13:44:18 by fabricebuyl       #+#    #+#              #
#    Updated: 2025/09/06 15:10:40 by fabricebuyl      ###   ########.fr        #
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
	main.cpp
	
OBJECTS = $(SOURCES:.cpp=.o)

all: $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I./headers -c $< -o $@

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I./headers -o $(NAME) $(OBJECTS)

clean:
	rm -f *.o

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re
