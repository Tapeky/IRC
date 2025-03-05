# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/01 11:54:57 by tsadouk           #+#    #+#              #
#    Updated: 2025/03/05 09:58:27 by tsadouk          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I./inc

SRCS  = src/main.cpp \
		src/Server.cpp \
		src/Client.cpp \
		src/Channel.cpp \
		src/commands/Command.cpp \
		src/commands/CommandExecutor.cpp \
		src/commands/CommandsAuth.cpp \
		src/commands/CommandsChannel.cpp \
		src/commands/CommandsOper.cpp \
		src/commands/CommandsMsg.cpp \
		src/commands/CommandsUtility.cpp \
		src/commands/CommandsBonus.cpp \
		bonus/src/FileTransfer.cpp \

OBJDIR = obj
OBJS = $(SRCS:src/%.cpp=$(OBJDIR)/%.o)
OBJ_SUBDIRS = $(sort $(dir $(OBJS)))

# Bonus
BOT_NAME = ircbot
BOT_CXXFLAGS = $(CXXFLAGS) -I./bonus/inc
BOT_SRCS = bonus/src/main.cpp \
		bonus/src/Bot.cpp \
		bonus/src/FileTransfer.cpp \

BOT_OBJDIR = $(OBJDIR)/bonus
BOT_OBJS = $(BOT_SRCS:bonus/src/%.cpp=$(BOT_OBJDIR)/%.o)
BOT_OBJ_SUBDIRS = $(sort $(dir $(BOT_OBJS)))

.SILENT: $(OBJS) $(NAME) $(BOT_OBJS) $(BOT_NAME)

all: $(NAME)
	@echo "Compilation Done! : $(NAME) is ready."

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJ_SUBDIRS): | $(OBJDIR)
	@mkdir -p $@

$(OBJDIR)/%.o: src/%.cpp | $(OBJ_SUBDIRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Bonus rules
bonus: $(NAME) $(BOT_NAME)
	@echo "Bonus compilation Done! : $(BOT_NAME) is ready."

$(BOT_OBJDIR):
	@mkdir -p $(BOT_OBJDIR)

$(BOT_OBJ_SUBDIRS): | $(BOT_OBJDIR)
	@mkdir -p $@

$(BOT_OBJDIR)/%.o: bonus/src/%.cpp | $(BOT_OBJ_SUBDIRS)
	$(CXX) $(BOT_CXXFLAGS) -c $< -o $@

$(BOT_NAME): $(BOT_OBJS)
	$(CXX) $(BOT_CXXFLAGS) $(BOT_OBJS) -o $(BOT_NAME)

clean:
	@rm -rf $(OBJDIR)

fclean: clean
	@rm -f $(NAME)
	@rm -f $(BOT_NAME)

re: fclean all

re_bonus: fclean bonus

.PHONY: all clean fclean re bonus re_bonus