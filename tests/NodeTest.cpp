/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeTest.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 18:23:49 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/17 16:52:50 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cassert>
#include <iostream>
#include <memory>

#include "../includes/Node.hpp"
#include "../includes/Edge.hpp" // Include the Edge header
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
					  << "Error: Expected DuplicateNodeException not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::DuplicateNodeException &e)
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
