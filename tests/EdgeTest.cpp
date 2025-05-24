#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "../includes/Edge.hpp"
#include "../includes/Event.hpp"
#include "../includes/colours.hpp"

bool testEdgeConstructorAndGetters()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testEdgeConstructorAndGetters..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		Edge edge("NodeA", "NodeB", 100, 80);
		if (edge.getNode1() != "NodeA" || edge.getNode2() != "NodeB"
			|| edge.getDistance() != 100 || edge.getSpeedLimit() != 80)
		{
			std::cerr << RED << "Error: Edge fields not set correctly" << RESET
					  << std::endl;
			testPassed = false;
		}
		std::cout << GREEN << "testEdgeConstructorAndGetters completed."
				  << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED
				  << "Exception in testEdgeConstructorAndGetters: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testEdgeAddEvent()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testEdgeAddEvent..." << RESET << std::endl;
	bool testPassed = true;
	try
	{
		Edge edge("NodeA", "NodeB", 50, 120);
		Event event("Track Blocked", 0.3f, std::chrono::seconds(3600), "edge");
		edge.addEvent(event);

		if (edge.getEvents().size() != 1)
		{
			std::cerr << RED << "Error: Event not added to edge" << RESET << std::endl;
			testPassed = false;
		}
		else if (edge.getEvents()[0].getEventName() != "Track Blocked")
		{
			std::cerr << RED << "Error: Event name mismatch" << RESET << std::endl;
			testPassed = false;
		}
		std::cout << GREEN << "testEdgeAddEvent completed." << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testEdgeAddEvent: " << e.what() << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testEdgeMultipleEvents()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testEdgeMultipleEvents..." << RESET << std::endl;
	bool testPassed = true;
	try
	{
		Edge edge("NodeA", "NodeB", 200, 90);
		for (int i = 0; i < 10; ++i)
		{
			Event event("Event" + std::to_string(i), 0.1f * i, std::chrono::seconds(100 + i), "edge");
			edge.addEvent(event);
		}
		if (edge.getEvents().size() != 10)
		{
			std::cerr << RED << "Error: Expected 10 events, got " << edge.getEvents().size() << RESET << std::endl;
			testPassed = false;
		}
		std::cout << GREEN << "testEdgeMultipleEvents completed." << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testEdgeMultipleEvents: " << e.what() << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

int main()
{
	bool allTestsPassed = true;
	allTestsPassed &= testEdgeConstructorAndGetters();
	allTestsPassed &= testEdgeAddEvent();
	allTestsPassed &= testEdgeMultipleEvents();

	if (allTestsPassed)
	{
		std::cout << GREEN << "All Edge tests passed." << RESET << std::endl;
	}
	else
	{
		std::cerr << RED << "Some Edge tests failed." << RESET << std::endl;
	}
	return allTestsPassed ? 0 : 1;
}
