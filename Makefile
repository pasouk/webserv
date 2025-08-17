# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fabricebuyl <fabricebuyl@student.42.fr>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/09 13:44:18 by fabricebuyl       #+#    #+#              #
#    Updated: 2025/08/17 13:43:42 by fabricebuyl      ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = webserv

SOURCES = QueryListener.cpp \
	ConfigParser.cpp \
	webserv.cpp
	
OBJECTS = $(SOURCES:.cpp=.o)

all: $(NAME)

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJECTS)

clean:
	rm -f *.o

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re
