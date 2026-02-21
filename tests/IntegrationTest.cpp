/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IntegrationTest.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 15:35:20 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "DijkstraPathfinding.hpp"
#include "InputHandler.hpp"
#include "Simulation.hpp"
#include "TestFramework.hpp"

static void cleanupResults()
{
	std::remove("output/results/TrainAB_14h10.result");
	std::remove("output/results/TrainAC_14h20.result");
	std::remove("output/results/TrainBA_14h24.result");
}

int main()
{
	Test::TestSuite suite("Integration");

	suite.run("full simulation runs without crash",
			  [](std::string &msg) {
				  cleanupResults();
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  ASSERT_TRUE(true, msg);
				  return true;
			  });

	suite.run("all trains arrive after simulation",
			  [](std::string &msg) {
				  cleanupResults();
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  std::vector<Train *> refs;
				  for (auto &t : data.trains)
					  refs.push_back(t.get());
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  for (auto *t : refs)
				  {
					  ASSERT_TRUE(t->hasArrived(), msg);
				  }
				  return true;
			  });

	suite.run("trains have non-zero arrival times",
			  [](std::string &msg) {
				  cleanupResults();
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  std::vector<Train *> refs;
				  for (auto &t : data.trains)
					  refs.push_back(t.get());
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  for (auto *t : refs)
				  {
					  ASSERT_TRUE(t->getCurrentTime() > 0.0, msg);
				  }
				  return true;
			  });

	suite.run("departure time <= arrival time for all trains",
			  [](std::string &msg) {
				  cleanupResults();
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  std::vector<Train *> refs;
				  for (auto &t : data.trains)
					  refs.push_back(t.get());
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  for (auto *t : refs)
				  {
					  ASSERT_TRUE(
						  t->getCurrentTime() >= t->getDepartureTime(),
						  msg);
				  }
				  return true;
			  });

	suite.run("paths have at least 2 nodes",
			  [](std::string &msg) {
				  cleanupResults();
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  std::vector<Train *> refs;
				  for (auto &t : data.trains)
					  refs.push_back(t.get());
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  for (auto *t : refs)
				  {
					  ASSERT_TRUE(t->getPath().size() >= 2, msg);
				  }
				  return true;
			  });

	suite.run("result files are created",
			  [](std::string &msg) {
				  cleanupResults();
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  std::ifstream f1("output/results/TrainAB_14h10.result");
				  ASSERT_TRUE(f1.is_open(), msg);
				  std::ifstream f2("output/results/TrainAC_14h20.result");
				  ASSERT_TRUE(f2.is_open(), msg);
				  std::ifstream f3("output/results/TrainBA_14h24.result");
				  ASSERT_TRUE(f3.is_open(), msg);
				  return true;
			  });

	suite.run("result file contains train name and travel time",
			  [](std::string &msg) {
				  cleanupResults();
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  std::ifstream f("output/results/TrainAB_14h10.result");
				  std::string line;
				  std::getline(f, line);
				  ASSERT_TRUE(line.find("TrainAB") != std::string::npos,
							  msg);
				  std::getline(f, line);
				  ASSERT_TRUE(
					  line.find("Final travel time") != std::string::npos,
					  msg);
				  return true;
			  });

	cleanupResults();
	return suite.summarize();
}
