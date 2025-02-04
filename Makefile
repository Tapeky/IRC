# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/01 11:54:57 by tsadouk           #+#    #+#              #
#    Updated: 2025/02/04 00:56:39 by tsadouk          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I./inc

SRCS = src/main.cpp \
       src/Server.cpp \
       src/Client.cpp \
       src/commands/Command.cpp \
       src/commands/CommandExecutor.cpp \
#       src/Channel.cpp

OBJDIR = obj
OBJS = $(SRCS:src/%.cpp=$(OBJDIR)/%.o)

OBJ_SUBDIRS = $(sort $(dir $(OBJS)))

.SILENT: $(OBJS) $(NAME)

all: $(NAME)
	@echo "Compilation Done ! : $(NAME) is ready."

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJ_SUBDIRS): | $(OBJDIR)
	@mkdir -p $@

$(OBJDIR)/%.o: src/%.cpp | $(OBJ_SUBDIRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean:
	@rm -rf $(OBJDIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re