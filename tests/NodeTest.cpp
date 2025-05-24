/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeTest.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 18:23:49 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 13:55:00 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>

#include "../includes/Edge.hpp"
#include "../includes/Event.hpp"
#include "../includes/Node.hpp"
#include "../includes/colours.hpp"

bool testAddEdge()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testAddEdge..." << RESET << std::endl;
	bool testPassed = true;
	try
	{
		Node node("A");
		Edge edge("A", "B", 10, 80);
		node.addEdge(edge);

		const auto &edges = node.getEdges();
		if (edges.size() != 1)
		{
			std::cerr << RED << "Error: Expected 1 edge, found " << edges.size()
					  << RESET << std::endl;
			testPassed = false;
		}
		else if (edges[0].getNode2() != "B" || edges[0].getDistance() != 10
				 || edges[0].getSpeedLimit() != 80)
		{
			std::cerr << RED << "Error: Edge fields not set correctly" << RESET
					  << std::endl;
			testPassed = false;
		}
		std::cout << GREEN << "testAddEdge completed." << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testAddEdge: " << e.what() << RESET
				  << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testAddEvent()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testAddEvent..." << RESET << std::endl;
	bool testPassed = true;
	try
	{
		Node  node("StationX");
		Event event("Riot", 0.2f, std::chrono::seconds(300), "node");
		node.addEvent(event);

		const auto &events = node.getEvents();
		if (events.size() != 1)
		{
			std::cerr << RED << "Error: Expected 1 event, found "
					  << events.size() << RESET << std::endl;
			testPassed = false;
		}
		else if (events[0].getEventName() != "Riot"
				 || events[0].getEventLocalization() != "node")
		{
			std::cerr << RED << "Error: Event fields not set correctly" << RESET
					  << std::endl;
			testPassed = false;
		}
		std::cout << GREEN << "testAddEvent completed." << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testAddEvent: " << e.what() << RESET
				  << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testDuplicateNodeName()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testDuplicateNodeName..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		Node node1("UniqueName");
		try
		{
			Node node2("UniqueName");
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
		std::cout << GREEN << "testDuplicateNodeName completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testDuplicateNodeName: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testEmptyNodeName()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testEmptyNodeName..." << RESET << std::endl;
	bool testPassed = true;
	try
	{
		try
		{
			Node node("");
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
		std::cout << GREEN << "testEmptyNodeName completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testEmptyNodeName: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

int main()
{
	bool allTestsPassed = true;
	allTestsPassed &= testAddEdge();
	allTestsPassed &= testAddEvent();
	allTestsPassed &= testDuplicateNodeName();
	allTestsPassed &= testEmptyNodeName();

	if (allTestsPassed)
	{
		std::cout << GREEN << "All Node tests passed." << RESET << std::endl;
	}
	else
	{
		std::cerr << RED << "Some Node tests failed." << RESET << std::endl;
	}
	return allTestsPassed ? 0 : 1;
}
