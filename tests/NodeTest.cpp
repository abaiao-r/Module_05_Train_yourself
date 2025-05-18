/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeTest.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 18:23:49 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/18 16:40:20 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cassert>
#include <iostream>
#include <memory>

#include "../includes/Edge.hpp"	 // Include the Edge header
#include "../includes/Node.hpp"
#include "../includes/colours.hpp"	// Include the colours header

bool testAddEdges()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testAddEdges..." << RESET << std::endl;

	bool testPassed = true;

	try
	{
		// Create nodes
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B");
		std::shared_ptr<Node> nodeC = std::make_shared<Node>("C");

		// Add edges
		nodeA->addEdge(nodeB, 10, 80);
		nodeA->addEdge(nodeC, 20, 90);

		// Verify edges
		auto edges = nodeA->getEdges();
		if (edges.size() != 2)
		{
			std::cerr << RED << "Error: Expected 2 edges, found "
					  << edges.size() << RESET << std::endl;
			testPassed = false;
		}

		bool foundEdgeB = false;
		bool foundEdgeC = false;

		for (const auto &edge : edges)
		{
			if (auto targetNode = edge.node.lock())
			{
				if (targetNode == nodeB && edge.distance == 10
					&& edge.speedLimit == 80)
				{
					foundEdgeB = true;
				}
				if (targetNode == nodeC && edge.distance == 20
					&& edge.speedLimit == 90)
				{
					foundEdgeC = true;
				}
			}
		}

		if (!foundEdgeB)
		{
			std::cerr << RED
					  << "Error: Edge to node B with distance 10 and speed "
						 "limit 80 not found"
					  << RESET << std::endl;
			testPassed = false;
		}
		if (!foundEdgeC)
		{
			std::cerr << RED
					  << "Error: Edge to node C with distance 20 and speed "
						 "limit 90 not found"
					  << RESET << std::endl;
			testPassed = false;
		}

		std::cout << GREEN << "testAddEdges completed." << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testAddEdges: " << e.what() << RESET
				  << std::endl;
		testPassed = false;
	}

	return (testPassed);
}

bool testDuplicateEdge()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testDuplicateEdge..." << RESET << std::endl;

	bool testPassed = true;

	try
	{
		// Create nodes
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A2");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B2");

		// Add edge
		nodeA->addEdge(nodeB, 10, 50);

		// Attempt to add a duplicate edge
		try
		{
			nodeA->addEdge(nodeB, 10, 50);
			std::cerr << RED
					  << "Error: Expected EdgeAlreadyExistsException not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::EdgeAlreadyExistsException &e)
		{
			std::cerr << GREEN << "Caught expected exception: " << e.what()
					  << RESET << std::endl;
		}

		std::cout << GREEN << "testDuplicateEdge completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testDuplicateEdge: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}

	return (testPassed);
}

bool testSelfEdge()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testSelfEdge..." << RESET << std::endl;

	bool testPassed = true;

	try
	{
		// Create node
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A3");

		// Attempt to add a self-edge
		try
		{
			nodeA->addEdge(nodeA, 5, 100);
			std::cerr << RED << "Error: Expected SelfEdgeException not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::SelfEdgeException &e)
		{
			std::cerr << GREEN << "Caught expected exception: " << e.what()
					  << RESET << std::endl;
		}

		std::cout << GREEN << "testSelfEdge completed." << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testSelfEdge: " << e.what() << RESET
				  << std::endl;
		testPassed = false;
	}

	return (testPassed);
}

bool testDuplicateNode()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testDuplicateNode..." << RESET << std::endl;

	bool testPassed = true;

	try
	{
		// Create node
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A4");

		// Attempt to create a node with a duplicate name
		try
		{
			std::shared_ptr<Node> nodeDuplicate = std::make_shared<Node>("A4");
			std::cerr << RED
					  << "Error: Expected InvalidNodeException not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidNodeException &e)
		{
			std::cerr << GREEN << "Caught expected exception: " << e.what()
					  << RESET << std::endl;
		}

		std::cout << GREEN << "testDuplicateNode completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testDuplicateNode: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}

	return (testPassed);
}

bool testInvalidEdgeZeroDistance()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEdgeZeroDistance..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A6a");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B6a");
		try
		{
			nodeA->addEdge(nodeB, 0, 50);
			std::cerr << RED
					  << "Error: Expected InvalidEdgeException for zero "
						 "distance not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidEdgeException &e)
		{
			std::cerr << GREEN << "Caught expected exception (zero distance): "
					  << e.what() << RESET << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << RED
				  << "Exception in testInvalidEdgeZeroDistance: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEdgeZeroSpeed()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEdgeZeroSpeed..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A6b");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B6b");
		try
		{
			nodeA->addEdge(nodeB, 10, 0);
			std::cerr << RED
					  << "Error: Expected InvalidEdgeException for zero speed "
						 "limit not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidEdgeException &e)
		{
			std::cerr << GREEN
					  << "Caught expected exception (zero speed): " << e.what()
					  << RESET << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << RED
				  << "Exception in testInvalidEdgeZeroSpeed: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEdgeNegativeDistance()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEdgeNegativeDistance..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A6c");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B6c");
		try
		{
			nodeA->addEdge(nodeB, -10, 50);
			std::cerr << RED
					  << "Error: Expected InvalidEdgeException for negative "
						 "distance not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidEdgeException &e)
		{
			std::cerr << GREEN
					  << "Caught expected exception (negative distance): "
					  << e.what() << RESET << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testInvalidEdgeNegativeDistance: "
				  << e.what() << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEdgeNegativeSpeed()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEdgeNegativeSpeed..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A6d");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B6d");
		try
		{
			nodeA->addEdge(nodeB, 10, -10);
			std::cerr << RED
					  << "Error: Expected InvalidEdgeException for negative "
						 "speed limit not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidEdgeException &e)
		{
			std::cerr << GREEN << "Caught expected exception (negative speed): "
					  << e.what() << RESET << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << RED
				  << "Exception in testInvalidEdgeNegativeSpeed: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEdgeDistanceOverflow()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEdgeDistanceOverflow..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A6e");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B6e");
		try
		{
			volatile size_t overflow = std::numeric_limits<size_t>::max();
			overflow++;
			nodeA->addEdge(nodeB, overflow, 50);
			std::cerr << RED
					  << "Error: Expected InvalidEdgeException for distance "
						 "overflow not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidEdgeException &e)
		{
			std::cerr << GREEN
					  << "Caught expected exception (distance overflow): "
					  << e.what() << RESET << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testInvalidEdgeDistanceOverflow: "
				  << e.what() << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEdgeSpeedOverflow()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEdgeSpeedOverflow..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A6f");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B6f");
		try
		{
			volatile size_t overflow = std::numeric_limits<size_t>::max();
			overflow++;
			nodeA->addEdge(nodeB, 10, overflow);
			std::cerr << RED
					  << "Error: Expected InvalidEdgeException for speed limit "
						 "overflow not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidEdgeException &e)
		{
			std::cerr << GREEN << "Caught expected exception (speed overflow): "
					  << e.what() << RESET << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << RED
				  << "Exception in testInvalidEdgeSpeedOverflow: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEdgeVeryNegativeDistance()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEdgeVeryNegativeDistance..."
			  << RESET << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A8a");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B8a");
		try
		{
			// Use a very negative value, e.g., INT64_MIN
			nodeA->addEdge(nodeB, static_cast<size_t>(INT64_MIN), 50);
			std::cerr << RED
					  << "Error: Expected InvalidEdgeException for very "
						 "negative distance not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidEdgeException &e)
		{
			std::cerr << GREEN
					  << "Caught expected exception (very negative distance): "
					  << e.what() << RESET << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testInvalidEdgeVeryNegativeDistance: "
				  << e.what() << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEdgeVeryNegativeSpeed()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEdgeVeryNegativeSpeed..."
			  << RESET << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A8b");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B8b");
		try
		{
			nodeA->addEdge(nodeB, 10, static_cast<size_t>(INT64_MIN));
			std::cerr << RED
					  << "Error: Expected InvalidEdgeException for very "
						 "negative speed not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidEdgeException &e)
		{
			std::cerr << GREEN
					  << "Caught expected exception (very negative speed): "
					  << e.what() << RESET << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testInvalidEdgeVeryNegativeSpeed: "
				  << e.what() << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEdgeVeryLargeNegativeDistance()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEdgeVeryLargeNegativeDistance..."
			  << RESET << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A9a");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B9a");
		try
		{
			// Use a very large negative value as a string literal and convert
			// to long double, then to size_t
			const char *bigNegStr
				= "-10000000000000000000000000000000000000000000000000000000000"
				  "000000000000000000000000000000000000000000000000000000000000"
				  "000000000000000000000000000000000000000000000000000000000000"
				  "000000000000000000000000000000000000000000000000000000000000"
				  "000000000000000000000000000000000000000000000000000000000000"
				  "000000000000000000000000000000000000000000000000000000000000"
				  "00000000000000000000000";
			long double bigNeg = std::strtold(bigNegStr, nullptr);
			size_t		val = static_cast<size_t>(bigNeg);
			nodeA->addEdge(nodeB, val, 50);
			std::cerr << RED
					  << "Error: Expected InvalidEdgeException for very large "
						 "negative distance not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidEdgeException &e)
		{
			std::cerr
				<< GREEN
				<< "Caught expected exception (very large negative distance): "
				<< e.what() << RESET << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << RED
				  << "Exception in testInvalidEdgeVeryLargeNegativeDistance: "
				  << e.what() << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testAddNodeWithNullName()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testAddNodeWithNullName..." << RESET
			  << std::endl;

	bool testPassed = true;

	try
	{
		const char *nullName = nullptr;
		try
		{
			// This will cause undefined behavior, but for the sake of the test:
			std::shared_ptr<Node> nodeNull = std::make_shared<Node>(
				nullName ? std::string(nullName) : std::string());
			std::cerr << RED
					  << "Error: Expected InvalidNodeException not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidNodeException &e)
		{
			std::cerr << GREEN << "Caught expected exception: " << e.what()
					  << RESET << std::endl;
		}
		std::cout << GREEN << "testAddNodeWithNullName completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testAddNodeWithNullName: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}

	return testPassed;
}

bool testAddNodeWithEmptyName()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testAddNodeWithEmptyName..." << RESET
			  << std::endl;

	bool testPassed = true;

	try
	{
		try
		{
			std::shared_ptr<Node> nodeEmpty = std::make_shared<Node>("");
			std::cerr << RED
					  << "Error: Expected InvalidNodeException not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidNodeException &e)
		{
			std::cerr << GREEN << "Caught expected exception: " << e.what()
					  << RESET << std::endl;
		}
		std::cout << GREEN << "testAddNodeWithEmptyName completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED
				  << "Exception in testAddNodeWithEmptyName: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}

	return testPassed;
}

void printNodeEdges(const std::shared_ptr<Node> &node)
{
	std::cout << CYAN << "Node " << node->getName() << " has edges to:" << RESET
			  << std::endl;
	for (const auto &edge : node->getEdges())
	{
		if (auto targetNode = edge.node.lock())
		{
			std::cout << CYAN << "  Node " << targetNode->getName()
					  << " with distance " << edge.distance
					  << " and speed limit " << edge.speedLimit << RESET
					  << std::endl;
		}
	}
}

int main(void)
{
	bool allTestsPassed = true;

	allTestsPassed &= testAddEdges();
	allTestsPassed &= testDuplicateEdge();
	allTestsPassed &= testSelfEdge();
	allTestsPassed &= testDuplicateNode();
	allTestsPassed &= testInvalidEdgeZeroDistance();
	allTestsPassed &= testInvalidEdgeZeroSpeed();
	allTestsPassed &= testInvalidEdgeNegativeDistance();
	allTestsPassed &= testInvalidEdgeNegativeSpeed();
	allTestsPassed &= testInvalidEdgeDistanceOverflow();
	allTestsPassed &= testInvalidEdgeSpeedOverflow();
	allTestsPassed &= testInvalidEdgeVeryNegativeDistance();
	allTestsPassed &= testInvalidEdgeVeryNegativeSpeed();
	allTestsPassed &= testInvalidEdgeVeryLargeNegativeDistance();
	allTestsPassed &= testAddNodeWithNullName();
	allTestsPassed &= testAddNodeWithEmptyName();

	// Create nodes for printing edges
	std::shared_ptr<Node> nodeA = std::make_shared<Node>("A5");
	std::shared_ptr<Node> nodeB = std::make_shared<Node>("B5");
	std::shared_ptr<Node> nodeC = std::make_shared<Node>("C5");

	// Add edges
	nodeA->addEdge(nodeB, 10, 80);
	nodeA->addEdge(nodeC, 20, 90);

	// Print node names and edges
	printNodeEdges(nodeA);

	// Final assertion
	if (allTestsPassed)
	{
		std::cout << GREEN << "All tests passed." << RESET << std::endl;
	}
	else
	{
		std::cerr << RED << "Some tests failed." << RESET << std::endl;
	}

	return (allTestsPassed ? 0 : 1);
}
