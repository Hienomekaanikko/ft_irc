# Project Name
NAME = ircserv

# Compiler and flags
CC 		= c++
STD 	= -std=c++17
CFLAGS 	= -Wall -Wextra -Werror ${STD} -MMD

# Header files
HEADERS = -I ./includes

# Directories
SRC_DIR = ./src
OBJ_DIR = ./obj

# Source files
SRCS = 	$(SRC_DIR)/main.cpp \
		$(SRC_DIR)/Server.cpp \
		$(SRC_DIR)/Client.cpp \
		${SRC_DIR}/Channel.cpp \
		$(SRC_DIR)/Messages.cpp
# 		$(SRC_DIR)/Parser.cpp \
# 		$(SRC_DIR)/Utils.cpp

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Targets
all: $(NAME)

# Build the executable
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $(HEADERS) -c $< -o $@

# Include dependency files
-include $(OBJS:.o=.d)

clean:
	@rm -rf $(OBJ_DIR)
	@echo "Objects directory and objects removed"

fclean: clean
	@rm -f $(NAME)
	@echo "Everything removed"

re: fclean all	

.PHONY: all clean fclean re
