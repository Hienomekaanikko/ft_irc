CC = c++
FLAGS = -Wall -Werror -Wextra
SRC = main.cpp Server.cpp Client.cpp Channel.cpp
OBJ = $(SRC:.cpp=.o)
NAME = ircserv
RM = rm -f

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

re: fclean all

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

.PHONY: all clean fclean re
