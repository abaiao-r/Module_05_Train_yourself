#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "../includes/Edge.hpp"
#include "../includes/Node.hpp"
#include "../includes/RailNetwork.hpp"
#include "../includes/colours.hpp"

static void printTestSuiteHeader()
{
	std::cout << CYAN << "------------------------------------------"
			  << std::endl;
	std::cout << "         RAIL NETWORK TEST SUITE         " << std::endl;
	std::cout << "------------------------------------------" << RESET
			  << std::endl
			  << std::endl;
}

static bool printResult(const std::string &testName, bool passed,
						const std::string &expected, const std::string &actual)
{
	std::cout << YELLOW << "Test: " << testName << RESET << std::endl;
	std::cout << "  Expected: " << expected << std::endl;
	std::cout << "  Actual: " << actual << std::endl;
	if (passed)
	{
		std::cout << GREEN << "  Result: PASS" << RESET << std::endl;
		return true;
	}
	else
	{
		std::cout << RED << "  Result: FAIL" << RESET << std::endl;
		return false;
	}
	std::cout << std::endl;
}

static bool testAddNode()
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
	return printResult("AddNode", found, "Node 'TestNode' in RailNetwork",
					   found ? "Node present" : "Node not found");
}

static bool testAddConnection()
{
	std::cout << BLUE << "[TEST] AddConnection" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto  nodeA = std::make_shared<Node>("NodeA");
	auto  nodeB = std::make_shared<Node>("NodeB");
	network.addConnection(nodeA, nodeB, 5, 100);  // Add speedLimit argument

	bool connectionExists = false;
	for (auto &neighbor : network.getNeighbours(nodeA))
	{
		if (neighbor.node.lock()->getName() == "NodeB"
			&& neighbor.distance == 5)
		{
			connectionExists = true;
			break;
		}
	}
	return printResult("AddConnection", connectionExists,
					   "Connection from NodeA to NodeB with distance 5",
					   connectionExists ? "Connection correct"
										: "Connection missing");
}

static bool testGetNodes()
{
	std::cout << BLUE << "[TEST] GetNodes" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	network.addNode(std::make_shared<Node>("NodeX"));
	network.addNode(std::make_shared<Node>("NodeY"));
	auto nodeCount = network.getNodes().size();
	bool correctCount = (nodeCount >= 2);
	return printResult("GetNodes", correctCount, "At least 2 nodes",
					   std::to_string(nodeCount) + " nodes");
}

static bool testGetNeighbours()
{
	std::cout << BLUE << "[TEST] GetNeighbours" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto  nodeA = std::make_shared<Node>("NodeC");
	auto  nodeB = std::make_shared<Node>("NodeD");
	network.addConnection(nodeA, nodeB, 7, 80);	 // Add speedLimit argument

	auto neighbours = network.getNeighbours(nodeA);
	bool foundB = false;
	for (auto &n : neighbours)
	{
		if (n.node.lock()->getName() == "NodeD" && n.distance == 7)
		{
			foundB = true;
			break;
		}
	}
	return printResult(
		"GetNeighbours", foundB, "Neighbor NodeD with distance 7",
		foundB ? "Found correct neighbor" : "Wrong/missing neighbor");
}

static bool testPrintNetwork()
{
	std::cout << BLUE << "[TEST] PrintNetwork" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto  nodeX = std::make_shared<Node>("PrintNodeX");
	auto  nodeY = std::make_shared<Node>("PrintNodeY");
	auto  nodeZ = std::make_shared<Node>("PrintNodeZ");
	network.addNode(nodeX);
	network.addNode(nodeY);
	network.addNode(nodeZ);
	network.addConnection(nodeX, nodeY, 10, 100);  // Add speedLimit argument
	network.addConnection(nodeX, nodeZ, 15, 100);  // Add speedLimit argument
	std::cout << "Testing printNetwork() (no validation, just ensure no crash):"
			  << std::endl;
	network.printNetwork();
	return true;
}

static bool testDuplicateNode()
{
	std::cout << BLUE << "[TEST] DuplicateNode" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	network.addNode(std::make_shared<Node>("DuplicateNode"));
	bool exceptionThrown = false;
	try
	{
		network.addNode(std::make_shared<Node>("DuplicateNode"));
	}
	catch (const Node::InvalidNodeException &)
	{
		exceptionThrown = true;
	}
	return printResult("DuplicateNode", exceptionThrown, "Exception thrown",
					   exceptionThrown ? "Exception thrown" : "No exception");
}

static bool testSelfEdge()
{
	std::cout << BLUE << "[TEST] SelfEdge" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto  node = std::make_shared<Node>("SelfEdgeNode");
	network.addNode(node);
	bool exceptionThrown = false;
	try
	{
		network.addConnection(node, node, 5, 100);	// Add speedLimit argument
	}
	catch (const Node::SelfEdgeException &)
	{
		exceptionThrown = true;
	}
	return printResult("SelfEdge", exceptionThrown, "Exception thrown",
					   exceptionThrown ? "Exception thrown" : "No exception");
}

static bool testEdgeAlreadyExists()
{
	std::cout << BLUE << "[TEST] EdgeAlreadyExists" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto  nodeA = std::make_shared<Node>("NodeA");
	auto  nodeB = std::make_shared<Node>("NodeB");
	network.addConnection(nodeA, nodeB, 5, 100);  // Add speedLimit argument
	bool exceptionThrown = false;
	try
	{
		network.addConnection(nodeA, nodeB, 5, 100);  // Add speedLimit argument
	}
	catch (const RailNetwork::ConnectionAlreadyExistsException &)
	{
		exceptionThrown = true;
	}
	return printResult("EdgeAlreadyExists", exceptionThrown, "Exception thrown",
					   exceptionThrown ? "Exception thrown" : "No exception");
}

static bool testNonExistentNode()
{
	std::cout << BLUE << "[TEST] NonExistentNode" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto  node = std::make_shared<Node>("NonExistentNode");
	bool  exceptionThrown = false;
	try
	{
		network.getNeighbours(node);
	}
	catch (const std::runtime_error &)
	{
		exceptionThrown = true;
	}
	return printResult("NonExistentNode", exceptionThrown, "Exception thrown",
					   exceptionThrown ? "Exception thrown" : "No exception");
}

static bool testEmptyNetwork()
{
	std::cout << BLUE << "[TEST] EmptyNetwork" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto  nodes = network.getNodes();
	bool  isEmpty = nodes.empty();
	return printResult("EmptyNetwork", isEmpty, "No nodes in network",
					   isEmpty ? "No nodes" : "Nodes present");
}

static bool testAddSameConnectionTwice()
{
	std::cout << BLUE << "[TEST] AddSameConnectionTwice" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	auto  nodeA = std::make_shared<Node>("SameConnA");
	auto  nodeB = std::make_shared<Node>("SameConnB");
	network.addConnection(nodeA, nodeB, 12, 90);
	bool exceptionThrown = false;
	try
	{
		network.addConnection(nodeA, nodeB, 12, 90);
	}
	catch (const RailNetwork::ConnectionAlreadyExistsException &)
	{
		exceptionThrown = true;
	}
	return printResult("AddSameConnectionTwice", exceptionThrown,
					   "Exception thrown",
					   exceptionThrown ? "Exception thrown" : "No exception");
}

static bool testAddConnectionWithNegativeDistance()
{
	std::cout << BLUE << "[TEST] AddConnectionWithNegativeDistance" << RESET
			  << std::endl;
	auto &network = RailNetwork::getInstance();
	auto  nodeA = std::make_shared<Node>("NegDistA");
	auto  nodeB = std::make_shared<Node>("NegDistB");
	network.addNode(nodeA);
	network.addNode(nodeB);
	bool exceptionThrown = false;
	try
	{
		network.addConnection(nodeA, nodeB, -5, 100);
	}
	catch (const Node::InvalidEdgeException &)
	{
		exceptionThrown = true;
	}
	catch (const std::invalid_argument &)
	{
		exceptionThrown = true;
	}
	if (!exceptionThrown)
	{
		std::cout
			<< YELLOW
			<< "[DEBUG] Printing network after negative distance connection:"
			<< RESET << std::endl;
		network.printNetwork();
	}
	return printResult("AddConnectionWithNegativeDistance", exceptionThrown,
					   "Exception thrown",
					   exceptionThrown ? "Exception thrown" : "No exception");
}

static bool testAddConnectionWithNegativeSpeed()
{
	std::cout << BLUE << "[TEST] AddConnectionWithNegativeSpeed" << RESET
			  << std::endl;
	auto &network = RailNetwork::getInstance();
	auto  nodeA = std::make_shared<Node>("NegSpeedA");
	auto  nodeB = std::make_shared<Node>("NegSpeedB");
	network.addNode(nodeA);
	network.addNode(nodeB);
	bool exceptionThrown = false;
	try
	{
		network.addConnection(nodeA, nodeB, 10, -50);
	}
	catch (const Node::InvalidEdgeException &)
	{
		exceptionThrown = true;
	}
	catch (const std::invalid_argument &)
	{
		exceptionThrown = true;
	}
	return printResult("AddConnectionWithNegativeSpeed", exceptionThrown,
					   "Exception thrown",
					   exceptionThrown ? "Exception thrown" : "No exception");
}

static bool testAddNodeWithEmptyName()
{
	std::cout << BLUE << "[TEST] AddNodeWithEmptyName" << RESET << std::endl;
	auto &network = RailNetwork::getInstance();
	bool  exceptionThrown = false;
	try
	{
		network.addNode(std::make_shared<Node>(""));
	}
	catch (const Node::InvalidNodeException &)
	{
		exceptionThrown = true;
	}
	catch (const std::invalid_argument &)
	{
		exceptionThrown = true;
	}
	return printResult("AddNodeWithEmptyName", exceptionThrown,
					   "Exception thrown",
					   exceptionThrown ? "Exception thrown" : "No exception");
}

/*  Helper function to run a test function in a child process
 *  Returns true if the test passed, false otherwise
 * This done to ensure that each test runs in isolation and does not affect the
 * state of other tests
 */
static bool runTest(bool (*testFunc)())
{
	pid_t pid = fork();
	if (pid < 0)
	{
		std::cerr << "Fork failed." << std::endl;
		return false;
	}
	if (pid == 0)
	{
		// Child process executes the test, then exits
		bool result = testFunc();
		_exit(result ? 0 : 1);
	}
	else
	{
		// Parent waits for the child to finish, then continues
		int status;
		waitpid(pid, &status, 0);
		return WIFEXITED(status) && WEXITSTATUS(status) == 0;
	}
}

int main(void)
{
	printTestSuiteHeader();
	bool allTestsPassed = true;
	allTestsPassed &= runTest(testAddNode);
	allTestsPassed &= runTest(testAddConnection);
	allTestsPassed &= runTest(testGetNodes);
	allTestsPassed &= runTest(testGetNeighbours);
	allTestsPassed &= runTest(testPrintNetwork);
	allTestsPassed &= runTest(testDuplicateNode);
	allTestsPassed &= runTest(testSelfEdge);
	allTestsPassed &= runTest(testEdgeAlreadyExists);
	allTestsPassed &= runTest(testNonExistentNode);
	allTestsPassed &= runTest(testEmptyNetwork);
	allTestsPassed &= runTest(testAddSameConnectionTwice);
	allTestsPassed &= runTest(testAddConnectionWithNegativeDistance);
	allTestsPassed &= runTest(testAddConnectionWithNegativeSpeed);
	allTestsPassed &= runTest(testAddNodeWithEmptyName);
	std::cout << (allTestsPassed ? GREEN : RED) << "All tests completed."
			  << RESET << std::endl;
	return allTestsPassed ? 0 : 1;
}