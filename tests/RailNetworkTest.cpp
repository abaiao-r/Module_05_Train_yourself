/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetworkTest.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 14:22:37 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 15:35:54 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <memory>
#include <chrono>

#include "../includes/Edge.hpp"
#include "../includes/Event.hpp"
#include "../includes/Node.hpp"
#include "../includes/RailNetwork.hpp"
#include "../includes/colours.hpp"

bool testRailNetworkAddAndPrint()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testRailNetworkAddAndPrint..." << RESET << std::endl;
	bool testPassed = true;

	try
	{
		auto nodeA = std::make_shared<Node>("A");
		auto nodeB = std::make_shared<Node>("B");
		auto nodeC = std::make_shared<Node>("C");

		RailNetwork::getInstance().addNode(nodeA);
		RailNetwork::getInstance().addNode(nodeB);
		RailNetwork::getInstance().addNode(nodeC);

		auto edgeAB = std::make_shared<Edge>("A", "B", 10, 80);
		auto edgeAC = std::make_shared<Edge>("A", "C", 20, 90);

		nodeA->addEdge(*edgeAB);
		nodeA->addEdge(*edgeAC);
		nodeB->addEdge(*edgeAB);
		nodeC->addEdge(*edgeAC);

		Event eventNodeA("Riot", 0.2f, std::chrono::seconds(300), "node");
		Event eventEdgeAB("Track Blocked", 0.3f, std::chrono::seconds(3600), "edge");
		nodeA->addEvent(eventNodeA);
		edgeAB->addEvent(eventEdgeAB);

		RailNetwork::getInstance().printNetwork();

		std::cout << GREEN << "testRailNetworkAddAndPrint completed." << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testRailNetworkAddAndPrint: " << e.what() << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

int main()
{
	bool allTestsPassed = true;
	allTestsPassed &= testRailNetworkAddAndPrint();

	if (allTestsPassed)
	{
		std::cout << GREEN << "All RailNetwork tests passed." << RESET << std::endl;
	}
	else
	{
		std::cerr << RED << "Some RailNetwork tests failed." << RESET << std::endl;
	}
	return allTestsPassed ? 0 : 1;
}