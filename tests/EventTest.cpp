#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>

#include "../includes/Event.hpp"
#include "../includes/Node.hpp"
#include "../includes/colours.hpp"

bool testValidEvent()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testValidEvent..." << RESET << std::endl;
	bool testPassed = true;
	try
	{
		Event e("TestEvent", 0.5f, std::chrono::seconds(60), "CityA");
		if (e.getEventName() != "TestEvent" || e.getEventProbability() != 0.5f
			|| e.getEventDuration() != std::chrono::seconds(60)
			|| e.getEventLocalization() != "CityA")
		{
			std::cerr << RED << "Error: Event fields not set correctly" << RESET
					  << std::endl;
			testPassed = false;
			return (testPassed);
		}
		std::cout << GREEN << "testValidEvent completed." << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testValidEvent: " << e.what() << RESET
				  << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEventName()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEventName..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		try
		{
			Event e("", 0.5f, std::chrono::seconds(60), "CityA");
			std::cerr << RED
					  << "Error: Expected InvalidEventNameException not thrown"
					  << RESET << std::endl;
			testPassed = false;
			return (testPassed);
		}
		catch (const Event::InvalidEventNameException &e)
		{
			std::cerr << GREEN << "Caught expected exception: " << e.what()
					  << RESET << std::endl;
		}
		std::cout << GREEN << "testInvalidEventName completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testInvalidEventName: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEventProbability()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEventProbability..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		try
		{
			Event e("E", -0.1f, std::chrono::seconds(60), "CityA");
			std::cerr
				<< RED
				<< "Error: Expected InvalidEventProbabilityException not thrown"
				<< RESET << std::endl;
			testPassed = false;
		}
		catch (const Event::InvalidEventProbabilityException &e)
		{
			std::cerr << GREEN
					  << "Caught expected exception (negative): " << e.what()
					  << RESET << std::endl;
		}
		try
		{
			Event e("E", 1.1f, std::chrono::seconds(60), "CityA");
			std::cerr
				<< RED
				<< "Error: Expected InvalidEventProbabilityException not thrown"
				<< RESET << std::endl;
			testPassed = false;
		}
		catch (const Event::InvalidEventProbabilityException &e)
		{
			std::cerr << GREEN << "Caught expected exception (greater than 1): "
					  << e.what() << RESET << std::endl;
		}
		std::cout << GREEN << "testInvalidEventProbability completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED
				  << "Exception in testInvalidEventProbability: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testInvalidEventDuration()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testInvalidEventDuration..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		try
		{
			Event e("E", 0.5f, std::chrono::seconds(0), "CityA");
			std::cerr
				<< RED
				<< "Error: Expected InvalidEventDurationException not thrown"
				<< RESET << std::endl;
			testPassed = false;
		}
		catch (const Event::InvalidEventDurationException &e)
		{
			std::cerr << GREEN
					  << "Caught expected exception (zero): " << e.what()
					  << RESET << std::endl;
		}
		try
		{
			Event e("E", 0.5f, std::chrono::seconds(-10), "CityA");
			std::cerr
				<< RED
				<< "Error: Expected InvalidEventDurationException not thrown"
				<< RESET << std::endl;
			testPassed = false;
		}
		catch (const Event::InvalidEventDurationException &e)
		{
			std::cerr << GREEN
					  << "Caught expected exception (negative): " << e.what()
					  << RESET << std::endl;
		}
		std::cout << GREEN << "testInvalidEventDuration completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED
				  << "Exception in testInvalidEventDuration: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testDuplicateEventNameOnNode()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testDuplicateEventNameOnNode..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> node = std::make_shared<Node>("CityA");
		node->addEvent("EventX", 0.5f, std::chrono::seconds(60));

		try
		{
			node->addEvent("EventX", 0.7f, std::chrono::seconds(120));
			std::cerr << RED
					  << "Error: Expected InvalidEventException not thrown"
					  << RESET << std::endl;
			testPassed = false;
		}
		catch (const Node::InvalidEventException &e)
		{
			std::cerr << GREEN
					  << "Caught expected exception (duplicate event name): "
					  << e.what() << RESET << std::endl;
		}
		std::cout << GREEN << "testDuplicateEventNameOnNode completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED
				  << "Exception in testDuplicateEventNameOnNode: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testExtremeProbability()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testExtremeProbability..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		Event e0("ZeroProb", 0.0f, std::chrono::seconds(10), "CityA");
		Event e1("OneProb", 1.0f, std::chrono::seconds(10), "CityA");
		std::cout << GREEN << "testExtremeProbability completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testExtremeProbability: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testLongEventName()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testLongEventName..." << RESET << std::endl;
	bool testPassed = true;
	try
	{
		std::string longName(1000, 'A');
		Event		e(longName, 0.5f, std::chrono::seconds(10), "CityA");
		if (e.getEventName() != longName)
		{
			std::cerr << RED << "Error: Long event name not set correctly"
					  << RESET << std::endl;
			testPassed = false;
		}
		std::cout << GREEN << "testLongEventName completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testLongEventName: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testManyEventsOnNode()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testManyEventsOnNode..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		std::shared_ptr<Node> node = std::make_shared<Node>("CityB");
		for (int i = 0; i < 100; ++i)
		{
			node->addEvent("Event" + std::to_string(i), 0.1f * (i % 10),
						   std::chrono::seconds(10 + i));
		}
		if (node->getEvents().size() != 100)
		{
			std::cerr << RED << "Error: Expected 100 events, got "
					  << node->getEvents().size() << RESET << std::endl;
			testPassed = false;
		}
		std::cout << GREEN << "testManyEventsOnNode completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testManyEventsOnNode: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

bool testDurationEdgeCases()
{
	std::cout << YELLOW << "-----------------------" << RESET << std::endl;
	std::cout << YELLOW << "Running testDurationEdgeCases..." << RESET
			  << std::endl;
	bool testPassed = true;
	try
	{
		try
		{
			Event e("Short", 0.5f, std::chrono::seconds(1), "CityA");
		}
		catch (...)
		{
			std::cerr << RED << "Error: Exception thrown for duration=1"
					  << RESET << std::endl;
			testPassed = false;
		}
		try
		{
			Event e("Long", 0.5f, std::chrono::seconds(1000000), "CityA");
		}
		catch (...)
		{
			std::cerr << RED << "Error: Exception thrown for large duration"
					  << RESET << std::endl;
			testPassed = false;
		}
		std::cout << GREEN << "testDurationEdgeCases completed." << RESET
				  << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "Exception in testDurationEdgeCases: " << e.what()
				  << RESET << std::endl;
		testPassed = false;
	}
	return testPassed;
}

int main()
{
	bool allTestsPassed = true;
	allTestsPassed &= testValidEvent();
	allTestsPassed &= testInvalidEventName();
	allTestsPassed &= testInvalidEventProbability();
	allTestsPassed &= testInvalidEventDuration();
	allTestsPassed &= testDuplicateEventNameOnNode();
	allTestsPassed &= testExtremeProbability();
	allTestsPassed &= testLongEventName();
	allTestsPassed &= testManyEventsOnNode();
	allTestsPassed &= testDurationEdgeCases();

	if (allTestsPassed)
	{
		std::cout << GREEN << "All Event tests passed." << RESET << std::endl;
	}
	else
	{
		std::cerr << RED << "Some Event tests failed." << RESET << std::endl;
	}
	return allTestsPassed ? 0 : 1;
}
