/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EdgeCaseTest.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 16:00:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by ctw03933         ###   ########.fr       */
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

static const std::string NET = "input/railNetworkPrintFolder/";
static const std::string TRN = "input/trainPrintFolder/";

static void cleanupResultFiles()
{
	std::remove("output/results/TrainAB_14h10.result");
	std::remove("output/results/TrainAC_14h10.result");
	std::remove("output/results/TrainAB_00h00.result");
	std::remove("output/results/TrainAB_23h59.result");
}

int main()
{
	Test::TestSuite suite("EdgeCase");

	/* ── Minimal network: 2 nodes, 1 rail, 1 train ── */

	suite.run("minimal network simulation completes",
			  [](std::string &msg) {
				  cleanupResultFiles();
				  auto data = InputHandler::loadData(
					  NET + "railNetworkMinimal.txt",
					  TRN + "trainPrintSingle.txt");
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  ASSERT_TRUE(true, msg);
				  return true;
			  });

	suite.run("minimal network train arrives",
			  [](std::string &msg) {
				  cleanupResultFiles();
				  auto data = InputHandler::loadData(
					  NET + "railNetworkMinimal.txt",
					  TRN + "trainPrintSingle.txt");
				  std::vector<Train *> refs;
				  for (auto &t : data.trains)
					  refs.push_back(t.get());
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  ASSERT_TRUE(refs[0]->hasArrived(), msg);
				  return true;
			  });

	suite.run("minimal network creates result file",
			  [](std::string &msg) {
				  cleanupResultFiles();
				  auto data = InputHandler::loadData(
					  NET + "railNetworkMinimal.txt",
					  TRN + "trainPrintSingle.txt");
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  std::ifstream f("output/results/TrainAB_14h10.result");
				  ASSERT_TRUE(f.is_open(), msg);
				  return true;
			  });

	/* ── Empty train file: network loads but no simulation ── */

	suite.run("empty train file runs without crash",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(
					  NET + "railNetworkMinimal.txt",
					  TRN + "trainPrintEmpty.txt");
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  ASSERT_TRUE(true, msg);
				  return true;
			  });

	/* ── Disconnected graph: train cannot reach destination ── */

	suite.run("disconnected graph handles unreachable train",
			  [](std::string &msg) {
				  cleanupResultFiles();
				  auto data = InputHandler::loadData(
					  NET + "railNetworkDisconnected.txt",
					  TRN + "trainPrintUnreachable.txt");
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  /* Should not crash — just logs error */
				  sim.run();
				  ASSERT_TRUE(true, msg);
				  return true;
			  });

	/* ── Midnight departure (00h00) ── */

	suite.run("midnight departure simulation completes",
			  [](std::string &msg) {
				  cleanupResultFiles();
				  auto data = InputHandler::loadData(
					  NET + "railNetworkMinimal.txt",
					  TRN + "trainPrintMidnight.txt");
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  std::ifstream f("output/results/TrainAB_00h00.result");
				  ASSERT_TRUE(f.is_open(), msg);
				  return true;
			  });

	/* ── Late night departure (23h59) ── */

	suite.run("late night departure simulation completes",
			  [](std::string &msg) {
				  cleanupResultFiles();
				  auto data = InputHandler::loadData(
					  NET + "railNetworkMinimal.txt",
					  TRN + "trainPrintLateNight.txt");
				  Simulation sim(
					  std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
				  sim.run();
				  std::ifstream f("output/results/TrainAB_23h59.result");
				  ASSERT_TRUE(f.is_open(), msg);
				  return true;
			  });

	/* ── Null pathfinder ── */

	suite.run("null pathfinder throws",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(
					  NET + "railNetworkMinimal.txt",
					  TRN + "trainPrintSingle.txt");
				  ASSERT_THROWS(
					  Simulation(std::move(data.network),
								 std::move(data.trains),
								 std::move(data.events), nullptr),
					  std::invalid_argument, msg);
				  return true;
			  });

	/* ── Train to unknown station ── */

	suite.run("train with unknown destination throws at load time",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkMinimal.txt",
						  TRN + "trainPrintBadStation.txt"),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	cleanupResultFiles();
	return suite.summarize();
}
