NAME		= Train

# Compiler
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -g

# Directories
SRCDIR		= src
OBJDIR		= objs
BINDIR		= bin
TESTDIR		= tests

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
			  -I$(SRCDIR)/FileOutputObserver

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
			  $(SRCDIR)/FileOutputObserver/FileOutputObserver.cpp

# Object files
OBJS		= $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

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
		$(BINDIR)/IntegrationTest

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

# ============================================================================ #
#                                   UTILITY                                    #
# ============================================================================ #

run: all
	./$(BINDIR)/$(NAME) input/railNetworkPrintFolder/railNetworkPrintGood.txt \
		input/trainPrintFolder/trainPrintGood.txt

.PHONY: all clean fclean re test run
