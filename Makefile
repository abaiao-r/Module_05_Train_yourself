NAME		= Train

# Compiler
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -g -MMD -MP

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
OBJS		= $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

# Auto-generated header dependencies
DEPS		= $(OBJS:.o=.d)
-include $(DEPS)

# ============================================================================ #
#                                    RULES                                     #
# ============================================================================ #

all: $(BINDIR)/$(NAME)

$(BINDIR)/$(NAME): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@
	@echo "\n$(NAME) built successfully ✓\n"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
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
		$(BINDIR)/EndToEndTest

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

$(BINDIR)/NodeTest: $(TESTDIR)/NodeTest.cpp $(OBJDIR)/Node/Node.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/RailNetworkTest: $(TESTDIR)/RailNetworkTest.cpp \
	$(OBJDIR)/Node/Node.o $(OBJDIR)/RailNetwork/RailNetwork.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/TrainTest: $(TESTDIR)/TrainTest.cpp \
	$(OBJDIR)/Train/Train.o $(OBJDIR)/Node/Node.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/EventTest: $(TESTDIR)/EventTest.cpp $(OBJDIR)/Event/Event.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/DijkstraTest: $(TESTDIR)/DijkstraTest.cpp \
	$(OBJDIR)/DijkstraPathfinding/DijkstraPathfinding.o \
	$(OBJDIR)/RailNetwork/RailNetwork.o $(OBJDIR)/Node/Node.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/InputHandlerTest: $(TESTDIR)/InputHandlerTest.cpp \
	$(OBJDIR)/InputHandler/InputHandler.o \
	$(OBJDIR)/RailNetwork/RailNetwork.o $(OBJDIR)/Node/Node.o \
	$(OBJDIR)/Event/Event.o $(OBJDIR)/Train/Train.o \
	$(OBJDIR)/TrainFactory/TrainFactory.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/TrainFactoryTest: $(TESTDIR)/TrainFactoryTest.cpp \
	$(OBJDIR)/TrainFactory/TrainFactory.o $(OBJDIR)/Train/Train.o \
	$(OBJDIR)/Node/Node.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/OutputManagerTest: $(TESTDIR)/OutputManagerTest.cpp \
	$(OBJDIR)/OutputManager/OutputManager.o \
	$(OBJDIR)/Train/Train.o $(OBJDIR)/Node/Node.o \
	$(OBJDIR)/Event/Event.o $(OBJDIR)/RailNetwork/RailNetwork.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

# Integration test needs all objects except main.o
LIB_OBJS = $(filter-out $(OBJDIR)/main.o, $(OBJS))

$(BINDIR)/IntegrationTest: $(TESTDIR)/IntegrationTest.cpp $(LIB_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/EdgeCaseTest: $(TESTDIR)/EdgeCaseTest.cpp $(LIB_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

$(BINDIR)/CombinationTest: $(TESTDIR)/CombinationTest.cpp $(LIB_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INC) -I$(TESTDIR) $^ -o $@

# E2E test only needs TestFramework.hpp — runs ./bin/Train as subprocess
$(BINDIR)/EndToEndTest: $(TESTDIR)/EndToEndTest.cpp $(BINDIR)/$(NAME)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -I$(TESTDIR) $< -o $@

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

# ============================================================================ #
#                               DEPENDENCIES                                   #
# ============================================================================ #

check-deps:
	@bash $(SCRIPTS)/check_deps.sh cli

check-deps-gui:
	@bash $(SCRIPTS)/check_deps.sh gui

deps:
	@bash $(SCRIPTS)/check_deps.sh install-cli

deps-gui:
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
	@bash $(SCRIPTS)/check_deps.sh gui
	@if [ -z "$(QMAKE)" ]; then \
		echo "\n\033[0;31m✗ qmake not found. Install Qt 6 first:\033[0m"; \
		echo "  \033[0;36mmake deps-gui\033[0m\n"; \
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
        check-deps check-deps-gui deps deps-gui \
        bonus bonus-clean run-animate run-multi run-gui
