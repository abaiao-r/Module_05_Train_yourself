#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../includes/RailNetwork.hpp"
#include "../includes/Node.hpp"
#include "../includes/colours.hpp"
#include <iostream>


static void printTestSuiteHeader()
{
	std::cout << CYAN << "------------------------------------------" << std::endl;
	std::cout << "         RAIL NETWORK TEST SUITE         " << std::endl;
	std::cout << "------------------------------------------" << RESET << std::endl << std::endl;
}

static void printResult(const std::string &testName, bool passed,
						const std::string &expected, const std::string &actual)
{
	std::cout << YELLOW << "Test: " << testName << RESET << std::endl;
	std::cout << "  Expected: " << expected << std::endl;
	std::cout << "  Actual: " << actual << std::endl;
	if (passed)
		std::cout << GREEN << "  Result: PASS" << RESET << std::endl;
	else
		std::cout << RED << "  Result: FAIL" << RESET << std::endl;
	std::cout << std::endl;
}

static void testAddNode()
{
	std::cout << BLUE << "[TEST] AddNode" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	network.addNode(std::make_shared<Node>("TestNode"));
	auto nodes = network.getNodes();
	bool found = false;
	for (auto &n : nodes)
	{
		if (n->getName() == "TestNode")
		{
			found = true;
			break;
		}
	}
	printResult("AddNode", found, "Node 'TestNode' in RailNetwork",
				found ? "Node present" : "Node not found");
}

static void testAddConnection()
{
	std::cout << BLUE << "[TEST] AddConnection" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto nodeA = std::make_shared<Node>("NodeA");
	auto nodeB = std::make_shared<Node>("NodeB");
	network.addConnection(nodeA, nodeB, 5);

	bool connectionExists = false;
	for (auto &neighbor : network.getNeighbours(nodeA))
	{
		if (neighbor.first->getName() == "NodeB" && neighbor.second == 5)
		{
			connectionExists = true;
			break;
		}
	}
	printResult("AddConnection", connectionExists,
				"Connection from NodeA to NodeB with distance 5",
				connectionExists ? "Connection correct" : "Connection missing");
}

static void testGetNodes()
{
	std::cout << BLUE << "[TEST] GetNodes" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	network.addNode(std::make_shared<Node>("NodeX"));
	network.addNode(std::make_shared<Node>("NodeY"));
	auto nodeCount = network.getNodes().size();
	bool correctCount = (nodeCount >= 2);
	printResult("GetNodes", correctCount, "At least 2 nodes",
				std::to_string(nodeCount) + " nodes");
}

static void testGetNeighbours()
{
	std::cout << BLUE << "[TEST] GetNeighbours" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto nodeA = std::make_shared<Node>("NodeC");
	auto nodeB = std::make_shared<Node>("NodeD");
	network.addConnection(nodeA, nodeB, 7);

	auto neighbours = network.getNeighbours(nodeA);
	bool foundB = false;
	for (auto &n : neighbours)
	{
		if (n.first->getName() == "NodeD" && n.second == 7)
		{
			foundB = true;
			break;
		}
	}
	printResult("GetNeighbours", foundB, "Neighbor NodeD with distance 7",
				foundB ? "Found correct neighbor" : "Wrong/missing neighbor");
}

static void testPrintNetwork()
{
	std::cout << BLUE << "[TEST] PrintNetwork" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto nodeX = std::make_shared<Node>("PrintNodeX");
	auto nodeY = std::make_shared<Node>("PrintNodeY");
    auto nodeZ = std::make_shared<Node>("PrintNodeZ");
	network.addNode(nodeX);
	network.addNode(nodeY);
    network.addNode(nodeZ);
	network.addConnection(nodeX, nodeY, 10);
    network.addConnection(nodeX, nodeZ, 15);
	std::cout << "Testing printNetwork() (no validation, just ensure no crash):" 
			  << std::endl;
	network.printNetwork();
}

static void testDuplicateNode()
{
	std::cout << BLUE << "[TEST] DuplicateNode" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	network.addNode(std::make_shared<Node>("DuplicateNode"));
	bool exceptionThrown = false;
	try
	{
		network.addNode(std::make_shared<Node>("DuplicateNode"));
	}
	catch (const Node::DuplicateNodeException &)
	{
		exceptionThrown = true;
	}
	printResult("DuplicateNode", exceptionThrown, "Exception thrown",
				exceptionThrown ? "Exception thrown" : "No exception");
}

static void testSelfEdge()
{
	std::cout << BLUE << "[TEST] SelfEdge" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto node = std::make_shared<Node>("SelfEdgeNode");
	network.addNode(node);
	bool exceptionThrown = false;
	try
	{
		network.addConnection(node, node, 5);
	}
	catch (const Node::SelfEdgeException &)
	{
		exceptionThrown = true;
	}
	printResult("SelfEdge", exceptionThrown, "Exception thrown",
				exceptionThrown ? "Exception thrown" : "No exception");
}

static void testEdgeAlreadyExists()
{
	std::cout << BLUE << "[TEST] EdgeAlreadyExists" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto nodeA = std::make_shared<Node>("NodeA");
	auto nodeB = std::make_shared<Node>("NodeB");
	network.addConnection(nodeA, nodeB, 5);
	bool exceptionThrown = false;
	try
	{
		network.addConnection(nodeA, nodeB, 5);
	}
	catch (const Node::EdgeAlreadyExistsException &)
	{
		exceptionThrown = true;
	}
	printResult("EdgeAlreadyExists", exceptionThrown, "Exception thrown",
				exceptionThrown ? "Exception thrown" : "No exception");
}

static void testNonExistentNode()
{
	std::cout << BLUE << "[TEST] NonExistentNode" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto node = std::make_shared<Node>("NonExistentNode");
	bool exceptionThrown = false;
	try
	{
		network.getNeighbours(node);
	}
	catch (const std::runtime_error &)
	{
		exceptionThrown = true;
	}
	printResult("NonExistentNode", exceptionThrown, "Exception thrown",
				exceptionThrown ? "Exception thrown" : "No exception");
}

static void testEmptyNetwork()
{
	std::cout << BLUE << "[TEST] EmptyNetwork" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto nodes = network.getNodes();
	bool isEmpty = nodes.empty();
	printResult("EmptyNetwork", isEmpty, "No nodes in network",
				isEmpty ? "No nodes" : "Nodes present");
}

static void runTest(void (*testFunc)())
{
	pid_t pid = fork();
	if (pid < 0)
	{
		std::cerr << "Fork failed." << std::endl;
		return;
	}
	if (pid == 0)
	{
		// Child process executes the test, then exits
		testFunc();
		_exit(0);
	}
	else
	{
		// Parent waits for the child to finish, then continues
		int status;
		waitpid(pid, &status, 0);
	}
}

int main()
{
	printTestSuiteHeader();
	runTest(testAddNode);
	runTest(testAddConnection);
	runTest(testGetNodes);
	runTest(testGetNeighbours);
	runTest(testPrintNetwork);
	runTest(testDuplicateNode);
	runTest(testSelfEdge);
	runTest(testEdgeAlreadyExists);
	runTest(testNonExistentNode);
	runTest(testEmptyNetwork);
	std::cout << GREEN << "All tests completed successfully." << RESET << std::endl;
	return 0;
}