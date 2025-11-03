NAME		= ircserv
CC			= c++
CFLAGS		= -Wall -Wextra -Werror -std=c++98
RM			= rm -rf

SRC_DIR		= src
INC_DIR		= inc
OBJ_DIR		= obj

SRCS		= $(SRC_DIR)/main.cpp \
			  $(SRC_DIR)/Server.cpp \
			  $(SRC_DIR)/Client.cpp \
			  $(SRC_DIR)/Channel.cpp \
			  $(SRC_DIR)/Commands.cpp \
			  $(SRC_DIR)/Utils.cpp

OBJS		= $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

INC			= -I$(INC_DIR)

all: $(NAME)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
