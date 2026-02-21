/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandlerTest.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 10:02:58 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "InputHandler.hpp"
#include "TestFramework.hpp"

int main()
{
	Test::TestSuite suite("InputHandler");

	suite.run("loads rail network file", [](std::string &msg) {
		auto data = InputHandler::loadData(
			"input/railNetworkPrintFolder/railNetworkPrintGood.txt",
			"input/trainPrintFolder/trainPrintGood.txt");
		ASSERT_EQ(10u, data.network.nodeCount(), msg);
		return true;
	});

	suite.run("loads all trains", [](std::string &msg) {
		auto data = InputHandler::loadData(
			"input/railNetworkPrintFolder/railNetworkPrintGood.txt",
			"input/trainPrintFolder/trainPrintGood.txt");
		ASSERT_EQ(3u, data.trains.size(), msg);
		return true;
	});

	suite.run("loads events including quoted names",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  ASSERT_EQ(5u, data.events.size(), msg);
				  return true;
			  });

	suite.run("train has correct departure time",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  // TrainAB departs at 14h10 = 14*3600 + 10*60 = 51000
				  ASSERT_TRUE(data.trains[0]->getDepartureTime() == 51000.0,
							  msg);
				  return true;
			  });

	suite.run("train has correct weight and friction",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  ASSERT_TRUE(data.trains[0]->getWeight() == 80.0, msg);
				  ASSERT_TRUE(
					  data.trains[0]->getFrictionCoefficient() == 0.05,
					  msg);
				  return true;
			  });

	suite.run("train has correct stop duration",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  // 00h10 = 600 seconds
				  ASSERT_TRUE(
					  data.trains[0]->getStopDuration() == 600.0, msg);
				  return true;
			  });

	suite.run("network has correct connections", [](std::string &msg) {
		auto data = InputHandler::loadData(
			"input/railNetworkPrintFolder/railNetworkPrintGood.txt",
			"input/trainPrintFolder/trainPrintGood.txt");
		auto &edges = data.network.getNeighbours("CityA");
		ASSERT_TRUE(edges.size() >= 2, msg);
		return true;
	});

	suite.run("throws on missing file", [](std::string &msg) {
		ASSERT_THROWS(
			InputHandler::loadData("nonexistent.txt", "also_missing.txt"),
			InputHandler::ParseException, msg);
		return true;
	});

	suite.run("throws on missing rail file only",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  "nonexistent.txt",
						  "input/trainPrintFolder/trainPrintGood.txt"),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on missing train file only",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  "input/railNetworkPrintFolder/"
						  "railNetworkPrintGood.txt",
						  "missing_trains.txt"),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	return suite.summarize();
}
