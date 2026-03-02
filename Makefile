# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/28 20:50:03 by ctw03933          #+#    #+#              #
#    Updated: 2026/03/01 18:37:08 by ctw03933         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= train_yourself

# Compiler
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -Wshadow -std=c++17 -g -MMD -MP

# Sanitizer flags — always enabled for tests, never for the main binary
SANFLAGS	= -fsanitize=address -fno-omit-frame-pointer

# make SANITIZE=1 → enable sanitizer on the main binary too (for CI leak checks)
ifeq ($(SANITIZE),1)
CXXFLAGS	+= -fsanitize=address -fno-omit-frame-pointer
LDFLAGS		+= -fsanitize=address
endif

# Directories
SRCDIR		= src
OBJDIR		= objs
BINDIR		= bin
TESTDIR		= tests
SCRIPTS		= scripts

# Include paths (one per class folder)
INC			= -I$(SRCDIR)/Edge \
			  -I$(SRCDIR)/Node \
			  -I$(SRCDIR)/RailNetwork \
			  -I$(SRCDIR)/Train \
			  -I$(SRCDIR)/Event \
			  -I$(SRCDIR)/TrainFactory \
			  -I$(SRCDIR)/IPathfinding \
			  -I$(SRCDIR)/DijkstraPathfinding \
			  -I$(SRCDIR)/InputHandler \
			  -I$(SRCDIR)/OutputManager \
			  -I$(SRCDIR)/Simulation \
			  -I$(SRCDIR)/ISimulationObserver \
			  -I$(SRCDIR)/FileOutputObserver \
			  -I$(SRCDIR)/GraphExporter \
			  -I$(SRCDIR)/TerminalAnimDisplay

# Source files
SRCS		= $(SRCDIR)/main.cpp \
			  $(SRCDIR)/Node/Node.cpp \
			  $(SRCDIR)/RailNetwork/RailNetwork.cpp \
			  $(SRCDIR)/Train/Train.cpp \
			  $(SRCDIR)/Event/Event.cpp \
			  $(SRCDIR)/TrainFactory/TrainFactory.cpp \
			  $(SRCDIR)/DijkstraPathfinding/DijkstraPathfinding.cpp \
			  $(SRCDIR)/InputHandler/InputHandler.cpp \
			  $(SRCDIR)/OutputManager/OutputManager.cpp \
			  $(SRCDIR)/Simulation/Simulation.cpp \
			  $(SRCDIR)/FileOutputObserver/FileOutputObserver.cpp \
			  $(SRCDIR)/GraphExporter/GraphExporter.cpp \
			  $(SRCDIR)/TerminalAnimDisplay/TerminalAnimDisplay.cpp

# Object files
OBJS		= $(OBJDIR)/main.o \
			  $(OBJDIR)/Node/Node.o \
			  $(OBJDIR)/RailNetwork/RailNetwork.o \
			  $(OBJDIR)/Train/Train.o \
			  $(OBJDIR)/Event/Event.o \
			  $(OBJDIR)/TrainFactory/TrainFactory.o \
			  $(OBJDIR)/DijkstraPathfinding/DijkstraPathfinding.o \
			  $(OBJDIR)/InputHandler/InputHandler.o \
			  $(OBJDIR)/OutputManager/OutputManager.o \
			  $(OBJDIR)/Simulation/Simulation.o \
			  $(OBJDIR)/FileOutputObserver/FileOutputObserver.o \
			  $(OBJDIR)/GraphExporter/GraphExporter.o \
			  $(OBJDIR)/TerminalAnimDisplay/TerminalAnimDisplay.o

# Auto-generated header dependencies
DEPS		= $(OBJDIR)/main.d \
			  $(OBJDIR)/Node/Node.d \
			  $(OBJDIR)/RailNetwork/RailNetwork.d \
			  $(OBJDIR)/Train/Train.d \
			  $(OBJDIR)/Event/Event.d \
			  $(OBJDIR)/TrainFactory/TrainFactory.d \
			  $(OBJDIR)/DijkstraPathfinding/DijkstraPathfinding.d \
			  $(OBJDIR)/InputHandler/InputHandler.d \
			  $(OBJDIR)/OutputManager/OutputManager.d \
			  $(OBJDIR)/Simulation/Simulation.d \
			  $(OBJDIR)/FileOutputObserver/FileOutputObserver.d \
			  $(OBJDIR)/GraphExporter/GraphExporter.d \
			  $(OBJDIR)/TerminalAnimDisplay/TerminalAnimDisplay.d
-include $(DEPS)

# ============================================================================ #
#                                    RULES                                     #
# ============================================================================ #

all: $(BINDIR)/$(NAME)

$(BINDIR)/$(NAME): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@
	@echo "\n$(NAME) built successfully ✓\n"

$(OBJDIR)/main.o: $(SRCDIR)/main.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/Node/Node.o: $(SRCDIR)/Node/Node.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/RailNetwork/RailNetwork.o: $(SRCDIR)/RailNetwork/RailNetwork.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/Train/Train.o: $(SRCDIR)/Train/Train.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/Event/Event.o: $(SRCDIR)/Event/Event.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/TrainFactory/TrainFactory.o: $(SRCDIR)/TrainFactory/TrainFactory.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/DijkstraPathfinding/DijkstraPathfinding.o: $(SRCDIR)/DijkstraPathfinding/DijkstraPathfinding.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/InputHandler/InputHandler.o: $(SRCDIR)/InputHandler/InputHandler.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/OutputManager/OutputManager.o: $(SRCDIR)/OutputManager/OutputManager.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/Simulation/Simulation.o: $(SRCDIR)/Simulation/Simulation.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/FileOutputObserver/FileOutputObserver.o: $(SRCDIR)/FileOutputObserver/FileOutputObserver.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/GraphExporter/GraphExporter.o: $(SRCDIR)/GraphExporter/GraphExporter.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/TerminalAnimDisplay/TerminalAnimDisplay.o: $(SRCDIR)/TerminalAnimDisplay/TerminalAnimDisplay.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
	rm -rf output

fclean: clean
	rm -rf $(BINDIR)

re: fclean all

# ============================================================================ #
#                                    TESTS                                     #
# ============================================================================ #

TESTS = $(BINDIR)/NodeTest \
		$(BINDIR)/RailNetworkTest \
		$(BINDIR)/TrainTest \
		$(BINDIR)/EventTest \
		$(BINDIR)/DijkstraTest \
		$(BINDIR)/InputHandlerTest \
		$(BINDIR)/TrainFactoryTest \
		$(BINDIR)/OutputManagerTest \
		$(BINDIR)/IntegrationTest \
		$(BINDIR)/EdgeCaseTest \
		$(BINDIR)/CombinationTest \
		$(BINDIR)/EndToEndTest \
		$(BINDIR)/InputComboTest \
		$(BINDIR)/CongestionTest

test: $(TESTS)
	@echo ""
	@./$(BINDIR)/NodeTest || exit 1
	@./$(BINDIR)/RailNetworkTest || exit 1
	@./$(BINDIR)/TrainTest || exit 1
	@./$(BINDIR)/EventTest || exit 1
	@./$(BINDIR)/DijkstraTest || exit 1
	@./$(BINDIR)/InputHandlerTest || exit 1
	@./$(BINDIR)/TrainFactoryTest || exit 1
	@./$(BINDIR)/OutputManagerTest || exit 1
	@./$(BINDIR)/IntegrationTest || exit 1
	@./$(BINDIR)/EdgeCaseTest || exit 1
	@./$(BINDIR)/CombinationTest || exit 1
	@./$(BINDIR)/EndToEndTest || exit 1
	@./$(BINDIR)/InputComboTest || exit 1
	@./$(BINDIR)/CongestionTest || exit 1

$(BINDIR)/NodeTest: $(TESTDIR)/NodeTest.cpp $(OBJDIR)/Node/Node.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/RailNetworkTest: $(TESTDIR)/RailNetworkTest.cpp \
	$(OBJDIR)/Node/Node.o $(OBJDIR)/RailNetwork/RailNetwork.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/TrainTest: $(TESTDIR)/TrainTest.cpp \
	$(OBJDIR)/Train/Train.o $(OBJDIR)/Node/Node.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/EventTest: $(TESTDIR)/EventTest.cpp $(OBJDIR)/Event/Event.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/DijkstraTest: $(TESTDIR)/DijkstraTest.cpp \
	$(OBJDIR)/DijkstraPathfinding/DijkstraPathfinding.o \
	$(OBJDIR)/RailNetwork/RailNetwork.o $(OBJDIR)/Node/Node.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/InputHandlerTest: $(TESTDIR)/InputHandlerTest.cpp \
	$(OBJDIR)/InputHandler/InputHandler.o \
	$(OBJDIR)/RailNetwork/RailNetwork.o $(OBJDIR)/Node/Node.o \
	$(OBJDIR)/Event/Event.o $(OBJDIR)/Train/Train.o \
	$(OBJDIR)/TrainFactory/TrainFactory.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/TrainFactoryTest: $(TESTDIR)/TrainFactoryTest.cpp \
	$(OBJDIR)/TrainFactory/TrainFactory.o $(OBJDIR)/Train/Train.o \
	$(OBJDIR)/Node/Node.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/OutputManagerTest: $(TESTDIR)/OutputManagerTest.cpp \
	$(OBJDIR)/OutputManager/OutputManager.o \
	$(OBJDIR)/Train/Train.o $(OBJDIR)/Node/Node.o \
	$(OBJDIR)/Event/Event.o $(OBJDIR)/RailNetwork/RailNetwork.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

# All objects except main.o
LIB_OBJS	= $(OBJDIR)/Node/Node.o \
			  $(OBJDIR)/RailNetwork/RailNetwork.o \
			  $(OBJDIR)/Train/Train.o \
			  $(OBJDIR)/Event/Event.o \
			  $(OBJDIR)/TrainFactory/TrainFactory.o \
			  $(OBJDIR)/DijkstraPathfinding/DijkstraPathfinding.o \
			  $(OBJDIR)/InputHandler/InputHandler.o \
			  $(OBJDIR)/OutputManager/OutputManager.o \
			  $(OBJDIR)/Simulation/Simulation.o \
			  $(OBJDIR)/FileOutputObserver/FileOutputObserver.o \
			  $(OBJDIR)/GraphExporter/GraphExporter.o \
			  $(OBJDIR)/TerminalAnimDisplay/TerminalAnimDisplay.o

$(BINDIR)/IntegrationTest: $(TESTDIR)/IntegrationTest.cpp $(LIB_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/EdgeCaseTest: $(TESTDIR)/EdgeCaseTest.cpp $(LIB_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/CombinationTest: $(TESTDIR)/CombinationTest.cpp $(LIB_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

# E2E test only needs TestFramework.hpp — runs ./bin/train_yourself as subprocess
$(BINDIR)/EndToEndTest: $(TESTDIR)/EndToEndTest.cpp $(BINDIR)/$(NAME)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) -I$(TESTDIR) $< -o $@

# Input combo test — runs ./bin/train_yourself against all input file pairs
$(BINDIR)/InputComboTest: $(TESTDIR)/InputComboTest.cpp $(BINDIR)/$(NAME)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) -I$(TESTDIR) $< -o $@

$(BINDIR)/CongestionTest: $(TESTDIR)/CongestionTest.cpp \
	$(OBJDIR)/DijkstraPathfinding/DijkstraPathfinding.o \
	$(OBJDIR)/RailNetwork/RailNetwork.o $(OBJDIR)/Node/Node.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

# ============================================================================ #
#                                   UTILITY                                    #
# ============================================================================ #

NETWORK	= input/railNetworkPrintFolder/railNetworkPrintGood.txt
TRAINS	= input/trainPrintFolder/trainPrintGood.txt

run: all
	./$(BINDIR)/$(NAME) $(NETWORK) $(TRAINS)

run-time: all
	./$(BINDIR)/$(NAME) $(NETWORK) $(TRAINS) --time

run-graph: all
	./$(BINDIR)/$(NAME) $(NETWORK) $(TRAINS) --graph network.dot

run-graph-time: all
	./$(BINDIR)/$(NAME) $(NETWORK) $(TRAINS) --time --graph network.dot

run-congestion: all
	./$(BINDIR)/$(NAME) $(NETWORK) $(TRAINS) --congestion

# ============================================================================ #
#                               DEPENDENCIES                                   #
# ============================================================================ #

setup:
	@bash $(SCRIPTS)/check_deps.sh install-gui

# ============================================================================ #
#                                   BONUS                                      #
# ============================================================================ #

# Auto-detect qmake: try qmake6, qmake, then common Homebrew paths
QMAKE := $(shell command -v qmake6 2>/dev/null \
         || command -v qmake 2>/dev/null \
         || ([ -x /opt/homebrew/bin/qmake ] && echo /opt/homebrew/bin/qmake) \
         || ([ -x /opt/homebrew/opt/qt/bin/qmake ] && echo /opt/homebrew/opt/qt/bin/qmake) \
         || ([ -x /usr/local/bin/qmake ] && echo /usr/local/bin/qmake) \
         || echo "")

bonus:
	@if [ -z "$(QMAKE)" ]; then \
		echo "\n\033[0;31m✗ qmake not found. Install Qt first:\033[0m"; \
		echo "  \033[0;36mmake setup\033[0m\n"; \
		exit 1; \
	fi
	@echo "\nBuilding Qt GUI bonus..."
	@mkdir -p objs/gui/moc
	@cd bonus/gui && $(QMAKE) bonus.pro && $(MAKE) -j4
	@echo "\nTrainGUI built successfully ✓  →  bin/TrainGUI\n"

bonus-clean:
	@cd bonus/gui && [ -f Makefile ] && $(MAKE) clean || true
	rm -rf objs/gui
	rm -f $(BINDIR)/TrainGUI

run-animate: all
	./$(BINDIR)/$(NAME) $(NETWORK) $(TRAINS) --animate

run-multi: all
	./$(BINDIR)/$(NAME) $(NETWORK) $(TRAINS) --runs 1000

run-gui: bonus
	@if [ "$$(uname)" = "Darwin" ]; then \
		open ./$(BINDIR)/TrainGUI.app; \
	else \
		./$(BINDIR)/TrainGUI; \
	fi

.PHONY: all clean fclean re test run run-time run-graph run-graph-time \
        setup \
        bonus bonus-clean run-animate run-multi run-gui
