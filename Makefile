# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/08/31 15:48:30 by andrefranci       #+#    #+#              #
#    Updated: 2024/10/24 19:42:43 by andrefranci      ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Compiler settings
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -Wshadow -std=c++14 -g #-fstandalone-debug #-fsanitize=address

# Directories
SRCDIR = src
OBJDIR = objs
BINDIR = bin
TESTDIR = tests

# Source Files
SRCS =	$(SRCDIR)/main.cpp \
        $(SRCDIR)/InputHandler.cpp \

# Object Files
OBJS = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Targets
NAME = Train

all: $(BINDIR)/$(NAME)

# Clean up object files
clean:
	rm -f $(OBJS)

# Clean up executable
fclean: clean
	rm -f $(BINDIR)/$(NAME)
	rm -rf $(BINDIR)

# Run the main program
run: fclean all
	$(BINDIR)/$(NAME)

# Rebuild everything
re: fclean all

# Valgrind for memory checking
valgrind: fclean all
	valgrind --leak-check=full --show-leak-kinds=all $(BINDIR)/$(NAME)

# Format the code (src and headers) according to the .clang-format file
format:
	find includes src -type f \( -name '*.hpp' -o -name '*.cpp' \) -exec clang-format -i -style=file {} +

# Build the main program
$(BINDIR)/$(NAME): $(OBJS)
	mkdir -p $(BINDIR)
	$(CXX) $(OBJS) $(CXXFLAGS) -o $(BINDIR)/$(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean fclean re run valgrind format
