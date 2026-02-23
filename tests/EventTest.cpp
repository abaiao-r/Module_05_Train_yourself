/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventTest.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Event.hpp"
#include "TestFramework.hpp"

int main()
{
	Test::TestSuite suite("Event");

	suite.run("constructor sets all fields", [](std::string &msg) {
		Event e("Riot", 0.05, 172800.0, "CityA");
		ASSERT_STR_EQ(std::string("Riot"), e.getName(), msg);
		ASSERT_TRUE(e.getProbability() == 0.05, msg);
		ASSERT_TRUE(e.getDuration() == 172800.0, msg);
		ASSERT_STR_EQ(std::string("CityA"), e.getNodeName(), msg);
		return true;
	});

	suite.run("probability 0 never triggers", [](std::string &msg) {
		Event e("Impossible", 0.0, 1000.0, "Node");
		std::mt19937 rng(42);
		for (int i = 0; i < 1000; i++)
		{
			ASSERT_FALSE(e.tryTrigger(rng), msg);
		}
		return true;
	});

	suite.run("probability 1 always triggers", [](std::string &msg) {
		Event e("Certain", 1.0, 500.0, "Node");
		std::mt19937 rng(42);
		for (int i = 0; i < 100; i++)
		{
			ASSERT_TRUE(e.tryTrigger(rng), msg);
		}
		return true;
	});

	suite.run("copy constructor works", [](std::string &msg) {
		Event original("Test", 0.5, 3600.0, "StationX");
		Event copy(original);
		ASSERT_STR_EQ(original.getName(), copy.getName(), msg);
		ASSERT_TRUE(copy.getProbability() == 0.5, msg);
		return true;
	});

	suite.run("probability 0.5 triggers roughly half",
			  [](std::string &msg) {
				  Event e("Maybe", 0.5, 100.0, "Node");
				  std::mt19937 rng(12345);
				  int triggers = 0;
				  const int trials = 10000;
				  for (int i = 0; i < trials; i++)
				  {
					  if (e.tryTrigger(rng))
						  triggers++;
				  }
				  double ratio = static_cast<double>(triggers) / trials;
				  ASSERT_TRUE(ratio > 0.45 && ratio < 0.55, msg);
				  return true;
			  });

	suite.run("throws on empty name", [](std::string &msg) {
		ASSERT_THROWS(Event("", 0.5, 100.0, "N"),
					  std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on negative probability", [](std::string &msg) {
		ASSERT_THROWS(Event("Bad", -0.1, 100.0, "N"),
					  std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on probability > 1", [](std::string &msg) {
		ASSERT_THROWS(Event("Bad", 1.1, 100.0, "N"),
					  std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on negative duration", [](std::string &msg) {
		ASSERT_THROWS(Event("Bad", 0.5, -1.0, "N"),
					  std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on empty node name", [](std::string &msg) {
		ASSERT_THROWS(Event("Bad", 0.5, 100.0, ""),
					  std::invalid_argument, msg);
		return true;
	});

	suite.run("rail event with two nodes", [](std::string &msg) {
		Event e("Storm", 0.3, 7200.0, "CityA", "CityB");
		ASSERT_TRUE(e.isRailEvent(), msg);
		ASSERT_STR_EQ(std::string("CityA"), e.getNodeName(), msg);
		ASSERT_STR_EQ(std::string("CityB"), e.getNodeName2(), msg);
		ASSERT_TRUE(e.getDuration() == 7200.0, msg);
		return true;
	});

	suite.run("node event has empty nodeName2", [](std::string &msg) {
		Event e("Riot", 0.1, 1000.0, "CityA");
		ASSERT_FALSE(e.isRailEvent(), msg);
		ASSERT_STR_EQ(std::string(""), e.getNodeName2(), msg);
		return true;
	});

	suite.run("rail event copy preserves nodeName2", [](std::string &msg) {
		Event original("Flood", 0.2, 3600.0, "NodeA", "NodeB");
		Event copy(original);
		ASSERT_TRUE(copy.isRailEvent(), msg);
		ASSERT_STR_EQ(std::string("NodeB"), copy.getNodeName2(), msg);
		return true;
	});

	return suite.summarize();
}
