/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IntegrationTest.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 04:01:26 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <memory>

#include "DijkstraPathfinding.hpp"
#include "InputHandler.hpp"
#include "Simulation.hpp"
#include "TestFramework.hpp"

int main()
{
	Test::TestSuite suite("Integration");

	suite.run("full simulation runs without crash",
			  [](std::string &msg) {
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
				  auto data = InputHandler::loadData(
					  "input/railNetworkPrintFolder/"
					  "railNetworkPrintGood.txt",
					  "input/trainPrintFolder/trainPrintGood.txt");
				  // Keep raw pointers before moving
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

	return suite.summarize();
}
