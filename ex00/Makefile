# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/08/31 15:48:30 by andrefranci       #+#    #+#              #
#    Updated: 2024/09/16 17:52:05 by abaiao-r         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Compiler settings
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -Wshadow -std=c++11 -g #-fstandalone-debug #-fsanitize=address
TEST_CXXFLAGS = -Wall -Wextra -Werror -Wshadow -std=c++11 -g -DTESTING # Different flags for tests

# Directories
SRCDIR = src
OBJDIR = objs
TESTDIR = tests

# Source Files
SRCS =	$(SRCDIR)/main.cpp \
		$(SRCDIR)/InputHandler.cpp

TEST_SRCS = $(TESTDIR)/SingletonTest.cpp

# Object Files
OBJS = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TEST_OBJS = $(TEST_SRCS:$(TESTDIR)/%.cpp=$(OBJDIR)/%.o)

# Targets
NAME = Train
TEST_NAME = SingletonTest

all: $(NAME)

# Clean up object files
clean:
	rm -rf $(OBJDIR)

# Clean up executable
fclean: clean
	rm -f $(NAME) $(TEST_NAME)

# Run the main program
run: fclean all
	./$(NAME)

# Rebuild everything
re: fclean all

# Valgrind for memory checking
valgrind: fclean all
	valgrind --leak-check=full --show-leak-kinds=all ./$(NAME)

# Test the Singleton implementation
testSingleton: fclean $(TEST_NAME)

# Build the main program
$(NAME): 	$(OBJS)
			$(CXX) $(OBJS) $(CXXFLAGS) -o  $(NAME) 
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build the test program
$(TEST_NAME): $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) $(TEST_CXXFLAGS) -o $(TEST_NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test files into object files
$(OBJDIR)/%.o: $(TESTDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

.PHONY: all clean fclean re run valgrind testSingleton
