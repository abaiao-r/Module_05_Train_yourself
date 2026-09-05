/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LiveMutationTest.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 00:00:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 00:00:00 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include "DijkstraPathfinding.hpp"
#include "InputHandler.hpp"
#include "Simulation.hpp"
#include "TestFramework.hpp"

static void cleanupResults(const std::vector<std::string> &names)
{
	for (const auto &n : names)
	{
		std::string path = "output/results/" + n + ".result";
		std::remove(path.c_str());
	}
}

static Simulation buildSim()
{
	auto data = InputHandler::loadData(
		"input/railNetworkPrintFolder/railNetworkPrintGood.txt",
		"input/trainPrintFolder/trainPrintGood.txt");
	return Simulation(std::move(data.network), std::move(data.trains),
					  std::move(data.events),
					  std::make_unique<DijkstraPathfinding>());
}

int main()
{
	Test::TestSuite suite("LiveMutation");

	suite.run("AddNode mutation is applied mid-run",
			  [](std::string &msg) {
				  cleanupResults({"TrainAB_14h10", "TrainAC_14h20",
								  "TrainBA_14h24"});
				  Simulation sim = buildSim();
				  sim.setQuiet(true);
				  bool enqueued = false;
				  sim.setAnimCallback(
					  [&](double, const std::vector<TrainState> &) {
						  if (!enqueued)
						  {
							  sim.enqueueMutation(
								  AddNodeCommand{"LiveDepot"});
							  enqueued = true;
						  }
					  });
				  sim.run();
				  auto names = sim.getNetwork().getNodeNames();
				  ASSERT_TRUE(std::find(names.begin(), names.end(),
										"LiveDepot")
								  != names.end(),
							  msg);
				  return true;
			  });

	suite.run("AddRail mutation is applied mid-run",
			  [](std::string &msg) {
				  cleanupResults({"TrainAB_14h10", "TrainAC_14h20",
								  "TrainBA_14h24"});
				  Simulation sim = buildSim();
				  sim.setQuiet(true);
				  bool enqueued = false;
				  sim.setAnimCallback(
					  [&](double, const std::vector<TrainState> &) {
						  if (!enqueued)
						  {
							  sim.enqueueMutation(
								  AddNodeCommand{"LiveDepot2"});
							  sim.enqueueMutation(AddRailCommand{
								  "CityA", "LiveDepot2", 3.0, 100.0});
							  enqueued = true;
						  }
					  });
				  sim.run();
				  const auto &edges = sim.getNetwork().getNeighbours("CityA");
				  bool found = false;
				  for (const auto &e : edges)
				  {
					  if (e.destination->getName() == "LiveDepot2")
						  found = true;
				  }
				  ASSERT_TRUE(found, msg);
				  return true;
			  });

	suite.run("AddTrain mutation with future departure is accepted",
			  [](std::string &msg) {
				  cleanupResults({"TrainAB_14h10", "TrainAC_14h20",
								  "TrainBA_14h24", "LiveTrain_14h10"});
				  Simulation sim = buildSim();
				  sim.setQuiet(true);
				  bool enqueued = false;
				  bool applied = false;
				  sim.setMutationCallbacks(
					  [&](const std::string &) { applied = true; },
					  [](const std::string &) {});
				  size_t trainsBefore = sim.getTrains().size();
				  sim.setAnimCallback(
					  [&](double simTime,
						  const std::vector<TrainState> &) {
						  if (!enqueued)
						  {
							  sim.enqueueMutation(AddTrainCommand{
								  "LiveTrain", 50.0, 0.05, 300.0, 30.0,
								  "CityA", "CityC", simTime + 30.0,
								  60.0});
							  enqueued = true;
						  }
					  });
				  sim.run();
				  ASSERT_TRUE(applied, msg);
				  ASSERT_EQ(trainsBefore + 1, sim.getTrains().size(), msg);
				  bool foundResult = false;
				  for (const auto &r : sim.getResults())
				  {
					  if (r.name == "LiveTrain")
						  foundResult = true;
				  }
				  ASSERT_TRUE(foundResult, msg);
				  return true;
			  });

	suite.run("AddTrain mutation with past departure is rejected",
			  [](std::string &msg) {
				  cleanupResults({"TrainAB_14h10", "TrainAC_14h20",
								  "TrainBA_14h24"});
				  Simulation sim = buildSim();
				  sim.setQuiet(true);
				  bool enqueued = false;
				  bool rejected = false;
				  std::string reason;
				  sim.setMutationCallbacks(
					  [](const std::string &) {},
					  [&](const std::string &r) {
						  rejected = true;
						  reason = r;
					  });
				  size_t trainsBefore = sim.getTrains().size();
				  sim.setAnimCallback(
					  [&](double simTime,
						  const std::vector<TrainState> &) {
						  if (!enqueued)
						  {
							  sim.enqueueMutation(AddTrainCommand{
								  "TooLateTrain", 50.0, 0.05, 300.0,
								  30.0, "CityA", "CityC",
								  simTime - 3600.0, 60.0});
							  enqueued = true;
						  }
					  });
				  sim.run();
				  ASSERT_TRUE(rejected, msg);
				  ASSERT_TRUE(reason.find("already passed")
									!= std::string::npos,
							  msg);
				  ASSERT_EQ(trainsBefore, sim.getTrains().size(), msg);
				  return true;
			  });

	suite.run("AddNode mutation rejects duplicate name",
			  [](std::string &msg) {
				  cleanupResults({"TrainAB_14h10", "TrainAC_14h20",
								  "TrainBA_14h24"});
				  Simulation sim = buildSim();
				  sim.setQuiet(true);
				  bool enqueued = false;
				  bool rejected = false;
				  sim.setMutationCallbacks(
					  [](const std::string &) {},
					  [&](const std::string &) { rejected = true; });
				  sim.setAnimCallback(
					  [&](double, const std::vector<TrainState> &) {
						  if (!enqueued)
						  {
							  sim.enqueueMutation(
								  AddNodeCommand{"CityA"});
							  enqueued = true;
						  }
					  });
				  sim.run();
				  ASSERT_TRUE(rejected, msg);
				  return true;
			  });

	suite.run("AddRail mutation rejects self-loop",
			  [](std::string &msg) {
				  cleanupResults({"TrainAB_14h10", "TrainAC_14h20",
								  "TrainBA_14h24"});
				  Simulation sim = buildSim();
				  sim.setQuiet(true);
				  bool enqueued = false;
				  bool rejected = false;
				  sim.setMutationCallbacks(
					  [](const std::string &) {},
					  [&](const std::string &) { rejected = true; });
				  sim.setAnimCallback(
					  [&](double, const std::vector<TrainState> &) {
						  if (!enqueued)
						  {
							  sim.enqueueMutation(AddRailCommand{
								  "CityA", "CityA", 5.0, 100.0});
							  enqueued = true;
						  }
					  });
				  sim.run();
				  ASSERT_TRUE(rejected, msg);
				  return true;
			  });

	suite.run("AddEvent mutation rejects unknown node",
			  [](std::string &msg) {
				  cleanupResults({"TrainAB_14h10", "TrainAC_14h20",
								  "TrainBA_14h24"});
				  Simulation sim = buildSim();
				  sim.setQuiet(true);
				  bool enqueued = false;
				  bool rejected = false;
				  sim.setMutationCallbacks(
					  [](const std::string &) {},
					  [&](const std::string &) { rejected = true; });
				  sim.setAnimCallback(
					  [&](double, const std::vector<TrainState> &) {
						  if (!enqueued)
						  {
							  sim.enqueueMutation(AddEventCommand{
								  "Ghost", 0.1, 60.0, "NoSuchNode", ""});
							  enqueued = true;
						  }
					  });
				  sim.run();
				  ASSERT_TRUE(rejected, msg);
				  return true;
			  });

	suite.run("AddEvent mutation is applied for a valid node",
			  [](std::string &msg) {
				  cleanupResults({"TrainAB_14h10", "TrainAC_14h20",
								  "TrainBA_14h24"});
				  Simulation sim = buildSim();
				  sim.setQuiet(true);
				  bool enqueued = false;
				  bool applied = false;
				  sim.setMutationCallbacks(
					  [&](const std::string &) { applied = true; },
					  [](const std::string &) {});
				  sim.setAnimCallback(
					  [&](double, const std::vector<TrainState> &) {
						  if (!enqueued)
						  {
							  sim.enqueueMutation(AddEventCommand{
								  "LiveEvent", 0.1, 60.0, "CityB", ""});
							  enqueued = true;
						  }
					  });
				  sim.run();
				  ASSERT_TRUE(applied, msg);
				  return true;
			  });

	suite.run("concurrent enqueue from another thread is not lost",
			  [](std::string &msg) {
				  cleanupResults({"TrainAB_14h10", "TrainAC_14h20",
								  "TrainBA_14h24"});
				  Simulation sim = buildSim();
				  sim.setQuiet(true);
				  std::atomic<int> applied{0};
				  sim.setMutationCallbacks(
					  [&](const std::string &) { applied++; },
					  [](const std::string &) {});

				  /* Spawn a producer thread that races against the
					 simulation thread's mutex-protected queue. Joining it
					 from within the first tick (rather than after run()
					 returns) keeps the test deterministic: run() has
					 hundreds of ticks left for this multi-minute journey,
					 so every enqueued mutation is guaranteed to be
					 drained on a later tick — while still exercising real
					 cross-thread access to enqueueMutation()'s mutex. */
				  bool joined = false;
				  sim.setAnimCallback(
					  [&](double, const std::vector<TrainState> &) {
						  if (joined)
							  return;
						  joined = true;
						  std::thread producer([&sim]() {
							  for (int i = 0; i < 25; i++)
								  sim.enqueueMutation(AddNodeCommand{
									  "Concurrent" + std::to_string(i)});
						  });
						  producer.join();
					  });
				  sim.run();

				  ASSERT_EQ(25, applied.load(), msg);
				  ASSERT_EQ(static_cast<size_t>(10 + 25),
							sim.getNetwork().nodeCount(), msg);
				  return true;
			  });

	return suite.summarize();
}
