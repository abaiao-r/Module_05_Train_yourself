/* Fleet route planning: the IRoutePlanner strategy. */

#include <memory>
#include <string>
#include <vector>

#include "DijkstraPathfinding.hpp"
#include "IndependentPlanner.hpp"
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

	return suite.summarize();
}
