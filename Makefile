# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#              #
#    Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME		= Train

# Compiler
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -g

# Directories
SRCDIR		= src
OBJDIR		= objs
BINDIR		= bin
TESTDIR		= tests

# Include paths (one per class folder + src root for colours.hpp)
INC			= -I$(SRCDIR) \
			  -I$(SRCDIR)/Edge \
			  -I$(SRCDIR)/Node \
			  -I$(SRCDIR)/Singleton \
			  -I$(SRCDIR)/RailNetwork \
			  -I$(SRCDIR)/Train \
			  -I$(SRCDIR)/TrainFactory \
			  -I$(SRCDIR)/InputHandler \
			  -I$(SRCDIR)/OutputManager \
			  -I$(SRCDIR)/Simulation \
			  -I$(SRCDIR)/Event \
			  -I$(SRCDIR)/EventManager \
			  -I$(SRCDIR)/IObserver \
			  -I$(SRCDIR)/IMediator \
			  -I$(SRCDIR)/IPathfindingAlgorithm \
			  -I$(SRCDIR)/AStarPathfinding \
			  -I$(SRCDIR)/DijkstraPathfinding

# Source files (add new .cpp files here as the project grows)
SRCS		= $(SRCDIR)/main.cpp \
			  $(SRCDIR)/Node/Node.cpp \
			  $(SRCDIR)/RailNetwork/RailNetwork.cpp \
			  $(SRCDIR)/InputHandler/InputHandler.cpp \
			  $(SRCDIR)/Train/Train.cpp \
			  $(SRCDIR)/TrainFactory/TrainFactory.cpp \
			  $(SRCDIR)/OutputManager/OutputManager.cpp \
			  $(SRCDIR)/Simulation/Simulation.cpp \
			  $(SRCDIR)/Event/Event.cpp \
			  $(SRCDIR)/EventManager/EventManager.cpp \
			  $(SRCDIR)/AStarPathfinding/AStarPathfinding.cpp \
			  $(SRCDIR)/DijkstraPathfinding/DijkstraPathfinding.cpp

# Object files (mirror source tree under objs/)
OBJS		= $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

# ============================================================================ #
#                                    RULES                                     #
# ============================================================================ #

all: $(BINDIR)/$(NAME)

# Link — only re-runs when .o files change (no relink)
$(BINDIR)/$(NAME): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@
	@echo "\n$(NAME) built successfully ✓\n"

# Compile — only recompiles changed sources (no relink)
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(BINDIR)

re: fclean all

# ============================================================================ #
#                                    TESTS                                     #
# ============================================================================ #

test: $(BINDIR)/SingletonTest $(BINDIR)/NodeTest $(BINDIR)/RailNetworkTest
	@echo ""
	@./$(BINDIR)/SingletonTest || exit 1
	@./$(BINDIR)/NodeTest || exit 1
	@./$(BINDIR)/RailNetworkTest || exit 1

$(BINDIR)/SingletonTest: $(TESTDIR)/SingletonTest.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $< -o $@

$(BINDIR)/NodeTest: $(TESTDIR)/NodeTest.cpp $(OBJDIR)/Node/Node.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/RailNetworkTest: $(TESTDIR)/RailNetworkTest.cpp $(OBJDIR)/Node/Node.o $(OBJDIR)/RailNetwork/RailNetwork.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

# ============================================================================ #
#                                   UTILITY                                    #
# ============================================================================ #

run: all
	./$(BINDIR)/$(NAME) input/railNetworkPrintFolder/railNetworkPrintGood.txt \
		input/trainPrintFolder/trainPrintGood.txt

format:
	@find $(SRCDIR) $(TESTDIR) -type f \( -name '*.hpp' -o -name '*.cpp' \) \
		-exec clang-format -i -style=file {} +

.PHONY: all clean fclean re test run format
