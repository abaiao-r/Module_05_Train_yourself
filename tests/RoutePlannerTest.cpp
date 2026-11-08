/* Fleet route planning: the IRoutePlanner strategy. */

#include <memory>
#include <string>
#include <vector>

#include "DijkstraPathfinding.hpp"
#include "IndependentPlanner.hpp"
#include "ReservationTable.hpp"
#include "SegmentTiming.hpp"
#include "Simulation.hpp"
#include "TrainFactory.hpp"
#include "RailNetwork.hpp"
#include "TestFramework.hpp"

/* A-B-D is 10 km + 10 km, A-C-D is 12 km + 12 km: B is the best route. */
static RailNetwork diamond()
{
	RailNetwork net;
	for (const char *n : {"A", "B", "C", "D"})
		net.addNode(n);
	net.addConnection("A", "B", 10.0, 100.0);
	net.addConnection("B", "D", 10.0, 100.0);
	net.addConnection("A", "C", 12.0, 100.0);
	net.addConnection("C", "D", 12.0, 100.0);
	return net;
}

static std::unique_ptr<Train> trainAD(const std::string &name,
									  const std::string &to = "D")
{
	return TrainFactory::createTrain(name, 80.0, 0.05, 356.0, 30.0, "A", to,
									 28800.0, 0.0);
}

static std::string names(const Route &r)
{
	std::string s;
	for (const auto &n : r)
		s += n->getName();
	return s;
}

/* Stub that sends everyone A -> C -> D instead of the shortest route. */
class ForcedViaC : public IRoutePlanner
{
  public:
	std::vector<Route> planAll(const FleetSnapshot &fleet) override
	{
		std::vector<Route> out;
		for (size_t i = 0; i < fleet.trains.size(); i++)
		{
			Route r = fleet.pathfinder.findPath("A", "C", fleet.network,
												PathWeightMode::Distance);
			Route tail = fleet.pathfinder.findPath("C", "D", fleet.network,
												   PathWeightMode::Distance);
			r.insert(r.end(), tail.begin() + 1, tail.end());
			out.push_back(r);
		}
		return out;
	}
	const char *name() const override { return "stub"; }
};

int main()
{
	Test::TestSuite suite("RoutePlanner");

	suite.run("IndependentPlanner equals the pathfinder in every mode",
			  [](std::string &msg) {
				  for (PathWeightMode m :
					   {PathWeightMode::Distance, PathWeightMode::Solo,
						PathWeightMode::Adaptive})
				  {
					  TrainFactory::resetIdCounter();
					  RailNetwork net = diamond();
					  DijkstraPathfinding dj;
					  auto t = trainAD("T1");
					  FleetSnapshot f{net, {t.get()}, m, {}, dj};
					  auto routes = IndependentPlanner().planAll(f);
					  ASSERT_EQ(1u, routes.size(), msg);
					  ASSERT_STR_EQ("ABD", names(routes[0]), msg);
					  ASSERT_STR_EQ("ABD",
									names(dj.findPath("A", "D", net, m)), msg);
				  }
				  return true;
			  });

	suite.run("two identical trains both get the same independent route",
			  [](std::string &msg) {
				  TrainFactory::resetIdCounter();
				  RailNetwork net = diamond();
				  DijkstraPathfinding dj;
				  auto t1 = trainAD("T1");
				  auto t2 = trainAD("T2");
				  FleetSnapshot f{net, {t1.get(), t2.get()},
								  PathWeightMode::Solo, {}, dj};
				  auto routes = IndependentPlanner().planAll(f);
				  ASSERT_EQ(2u, routes.size(), msg);
				  ASSERT_STR_EQ("ABD", names(routes[0]), msg);
				  ASSERT_STR_EQ("ABD", names(routes[1]), msg);
				  return true;
			  });

	suite.run("unreachable destination keeps the exact error text",
			  [](std::string &msg) {
				  TrainFactory::resetIdCounter();
				  RailNetwork net = diamond();
				  net.addNode("Z");
				  DijkstraPathfinding dj;
				  auto t = trainAD("Lost", "Z");
				  FleetSnapshot f{net, {t.get()}, PathWeightMode::Solo, {}, dj};
				  std::string what;
				  try
				  {
					  IndependentPlanner().planAll(f);
				  }
				  catch (const std::runtime_error &e)
				  {
					  what = e.what();
				  }
				  ASSERT_TRUE(what.rfind("No path found for Lost", 0) == 0, msg);
				  return true;
			  });

	suite.run("Simulation uses the injected planner and defaults to independent",
			  [](std::string &msg) {
				  for (int injected = 0; injected < 2; injected++)
				  {
					  TrainFactory::resetIdCounter();
					  std::vector<std::unique_ptr<Train>> trains;
					  trains.push_back(trainAD("T1"));
					  Train *raw = trains[0].get();
					  Simulation sim(diamond(), std::move(trains), {},
									 std::make_unique<DijkstraPathfinding>(),
									 PathWeightMode::Solo);
					  sim.setQuiet(true);
					  if (injected)
						  sim.setRoutePlanner(std::make_unique<ForcedViaC>());
					  sim.run();
					  ASSERT_STR_EQ(injected ? "ACD" : "ABD",
									names(raw->getPath()), msg);
				  }
				  return true;
			  });

	suite.run("reservation: opposite direction conflicts, same direction does not",
			  [](std::string &msg) {
				  ReservationTable t;
				  t.reserve("A", "B", 0, 100, 200);
				  ASSERT_FALSE(t.isFree("B", "A", 0, 150, 160), msg);
				  ASSERT_TRUE(t.isFree("A", "B", 0, 150, 160), msg);
				  ASSERT_TRUE(t.isFree("B", "A", 1, 150, 160), msg);
				  return true;
			  });

	suite.run("reservation: windows are half-open",
			  [](std::string &msg) {
				  ReservationTable t;
				  t.reserve("A", "B", 0, 100, 200);
				  ASSERT_TRUE(t.isFree("B", "A", 0, 200, 300), msg);
				  ASSERT_FALSE(t.isFree("B", "A", 0, 199, 300), msg);
				  ASSERT_TRUE(t.isFree("B", "A", 0, 0, 100), msg);
				  return true;
			  });

	suite.run("reservation: earliestFree waits for the exact end of the blocker",
			  [](std::string &msg) {
				  ReservationTable t;
				  t.reserve("A", "B", 0, 100, 200);
				  ASSERT_EQ(200.0, t.earliestFree("B", "A", 0, 150, 20), msg);
				  ASSERT_EQ(150.0, t.earliestFree("A", "B", 0, 150, 20), msg);
				  ASSERT_EQ(150.0, t.earliestFree("B", "A", 1, 150, 20), msg);
				  t.reserve("A", "B", 0, 200, 260);
				  ASSERT_EQ(260.0, t.earliestFree("B", "A", 0, 150, 20), msg);
				  return true;
			  });

	suite.run("reservation: legacy mode (no head-on exclusion) never conflicts",
			  [](std::string &msg) {
				  ReservationTable t(false);
				  t.reserve("A", "B", 0, 100, 200);
				  ASSERT_TRUE(t.isFree("B", "A", 0, 150, 160), msg);
				  ASSERT_EQ(150.0, t.earliestFree("B", "A", 0, 150, 20), msg);
				  return true;
			  });

	suite.run("reservation: same-direction overlap is measured in seconds",
			  [](std::string &msg) {
				  ReservationTable t;
				  t.reserve("A", "B", 0, 100, 200);
				  ASSERT_EQ(50.0, t.sameDirectionOverlap("A", "B", 0, 150, 400),
							msg);
				  ASSERT_EQ(0.0, t.sameDirectionOverlap("B", "A", 0, 150, 400),
							msg);
				  ASSERT_EQ(0.0, t.sameDirectionOverlap("A", "B", 1, 150, 400),
							msg);
				  return true;
			  });

	suite.run("free-flow seconds: 10 km at 100 km/h is 360 s, lane speed wins",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("A");
				  net.addNode("B");
				  net.addConnection("A", "B", 10.0, 100.0, 2, {100.0, 50.0});
				  const Edge &e = net.getNeighbours("A")[0];
				  ASSERT_EQ(360.0, freeFlowSeconds(e), msg);
				  ASSERT_EQ(360.0, freeFlowSeconds(e, 0), msg);
				  ASSERT_EQ(720.0, freeFlowSeconds(e, 1), msg);
				  return true;
			  });

	return suite.summarize();
}
