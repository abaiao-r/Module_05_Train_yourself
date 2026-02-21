/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventTest.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r         ###   ########.fr       */
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

	return suite.summarize();
}
