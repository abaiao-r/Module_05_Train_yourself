/* Fleet route planning: the IRoutePlanner strategy. */

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "DijkstraPathfinding.hpp"
#include "CbsPlanner.hpp"
#include "CooperativeCost.hpp"
#include "IndependentPlanner.hpp"
#include "PrioritizedPlanner.hpp"
#include "RefinedPlanner.hpp"
#include "ReplanPolicy.hpp"
#include "ReservationTable.hpp"
#include "SegmentTiming.hpp"
#include "Simulation.hpp"
#include "TrainFactory.hpp"
#include "RailNetwork.hpp"
#include "TestFramework.hpp"

/* A-B-D is 10 km + 10 km; A-C-D is 2 x cKm. With the default 12 km, B
   is clearly the best route. */
static RailNetwork diamond(double cKm = 12.0)
{
	RailNetwork net;
	for (const char *n : {"A", "B", "C", "D"})
		net.addNode(n);
	net.addConnection("A", "B", 10.0, 100.0);
	net.addConnection("B", "D", 10.0, 100.0);
	net.addConnection("A", "C", cKm, 100.0);
	net.addConnection("C", "D", cKm, 100.0);
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

static std::unique_ptr<Train> trainAt(const std::string &name, double dep,
									  const std::string &from = "A",
									  const std::string &to = "D")
{
	return TrainFactory::createTrain(name, 80.0, 0.05, 356.0, 30.0, from, to,
									 dep, 0.0);
}

/* Total actual seconds of the fleet on the diamond with a given planner. */
static double simulate(std::unique_ptr<IRoutePlanner> planner,
					   std::vector<std::unique_ptr<Train>> trains,
					   std::map<std::string, std::string> *routes = nullptr)
{
	std::vector<const Train *> raw;
	for (auto &t : trains)
		raw.push_back(t.get());
	Simulation sim(diamond(10.0), std::move(trains), {},
				   std::make_unique<DijkstraPathfinding>(),
				   PathWeightMode::Solo);
	sim.setQuiet(true);
	sim.setRoutePlanner(std::move(planner));
	sim.run();
	double total = 0;
	for (const auto &r : sim.getResults())
		total += r.actualTime;
	if (routes)
		for (const Train *t : raw)
			(*routes)[t->getName()] = names(t->getPath());
	return total;
}

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

	/* A-B-D = 720 s and A-C-D = 756 s free-flow; every same-way train
	   already on a rail adds 20 s. */
	auto routeWith = [](const ReservationTable &table, bool lanes = false) {
		RailNetwork net = diamond(10.5);
		DijkstraPathfinding dj;
		SegmentEventRisk risk = tollMap(table, net, "A", 0.0, 0.0, lanes);
		return names(dj.findConstrainedPath("A", "D", net, {}, risk, 0.0));
	};

	suite.run("toll: empty table adds nothing and the free-flow route wins",
			  [&](std::string &msg) {
				  ReservationTable t;
				  RailNetwork net = diamond(10.5);
				  ASSERT_TRUE(tollMap(t, net, "A", 0.0, 0.0, false).empty(), msg);
				  ASSERT_STR_EQ("ABD", routeWith(t), msg);
				  return true;
			  });

	suite.run("toll: the detour wins once the toll passes its 36 s extra cost",
			  [&](std::string &msg) {
				  ReservationTable one;
				  one.reserve("B", "D", 0, 360, 720); // toll 20 s: 740 < 756
				  ASSERT_STR_EQ("ABD", routeWith(one), msg);
				  ReservationTable two = one;
				  two.reserve("B", "D", 0, 400, 760); // toll 40 s: 760 > 756
				  ASSERT_STR_EQ("ACD", routeWith(two), msg);
				  return true;
			  });

	suite.run("toll: opposite traffic is priced as the exact wait",
			  [&](std::string &msg) {
				  RailNetwork net = diamond(10.5);
				  const Edge &bd = net.getNeighbours("B")[1];
				  ReservationTable t;
				  t.reserve("D", "B", 0, 300, 800);
				  ASSERT_EQ(440.0, segmentToll(t, "B", bd, 360.0, true), msg);
				  ReservationTable legacy(false);
				  legacy.reserve("D", "B", 0, 300, 800);
				  ASSERT_EQ(0.0, segmentToll(legacy, "B", bd, 360.0, false), msg);
				  return true;
			  });

	suite.run("toll: a free parallel lane costs 0, all lanes blocked wait",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("B");
				  net.addNode("D");
				  net.addConnection("B", "D", 10.0, 100.0, 2);
				  const Edge &e = net.getNeighbours("B")[0];
				  ReservationTable t;
				  t.reserve("D", "B", 0, 300, 800);
				  auto one = bestLane(t, "B", e, 360.0, true);
				  ASSERT_EQ(1, one.first, msg);
				  ASSERT_EQ(0.0, one.second, msg);
				  t.reserve("D", "B", 1, 300, 900);
				  auto both = bestLane(t, "B", e, 360.0, true);
				  ASSERT_EQ(0, both.first, msg);
				  ASSERT_EQ(440.0, both.second, msg); // lane 0 frees at 800
				  return true;
			  });

	suite.run("toll: never negative or infinite, whatever is reserved",
			  [](std::string &msg) {
				  RailNetwork net = diamond();
				  ReservationTable t;
				  for (int i = 0; i < 20; i++)
				  {
					  t.reserve(i % 2 ? "A" : "B", i % 2 ? "B" : "A", 0,
							    i * 37.0, i * 37.0 + 500.0);
					  t.reserve("C", "D", 0, i * 11.0, i * 11.0 + 90.0);
				  }
				  for (bool lanes : {false, true})
					  for (const auto &kv : tollMap(t, net, "A", 0.0, 60.0, lanes))
					  {
						  ASSERT_TRUE(kv.second > 0.0, msg);
						  ASSERT_TRUE(kv.second < 1e9, msg);
					  }
				  return true;
			  });

	suite.run("prioritized: first train keeps the best route, second is sent round",
			  [](std::string &msg) {
				  TrainFactory::resetIdCounter();
				  RailNetwork net = diamond(10.0);
				  DijkstraPathfinding dj;
				  auto t1 = trainAt("T1", 28800.0);
				  auto t2 = trainAt("T2", 28800.0);
				  FleetSnapshot f{net, {t1.get(), t2.get()},
								  PathWeightMode::Solo, {}, dj};
				  auto routes = PrioritizedPlanner().planAll(f);
				  ASSERT_STR_EQ("ABD", names(routes[0]), msg);
				  ASSERT_STR_EQ("ACD", names(routes[1]), msg);
				  return true;
			  });

	suite.run("prioritized: priority follows departure time, ties follow name",
			  [](std::string &msg) {
				  TrainFactory::resetIdCounter();
				  RailNetwork net = diamond(10.0);
				  DijkstraPathfinding dj;
				  auto late = trainAt("Aaa", 28810.0);
				  auto early = trainAt("Zzz", 28800.0);
				  FleetSnapshot f{net, {late.get(), early.get()},
								  PathWeightMode::Solo, {}, dj};
				  auto routes = PrioritizedPlanner().planAll(f);
				  ASSERT_STR_EQ("ACD", names(routes[0]), msg); // late: round
				  ASSERT_STR_EQ("ABD", names(routes[1]), msg); // early: best
				  auto x = trainAt("Zzz", 28800.0);
				  auto y = trainAt("Aaa", 28800.0);
				  FleetSnapshot g{net, {x.get(), y.get()},
								  PathWeightMode::Solo, {}, dj};
				  auto tie = PrioritizedPlanner().planAll(g);
				  ASSERT_STR_EQ("ACD", names(tie[0]), msg); // Zzz plans second
				  ASSERT_STR_EQ("ABD", names(tie[1]), msg); // Aaa plans first
				  return true;
			  });

	suite.run("prioritized: a train planned alone gets the independent route",
			  [](std::string &msg) {
				  TrainFactory::resetIdCounter();
				  RailNetwork net = diamond(10.0);
				  DijkstraPathfinding dj;
				  auto t1 = trainAt("T1", 28800.0);
				  auto t2 = trainAt("T2", 28800.0);
				  auto t3 = trainAt("T3", 28800.0);
				  FleetSnapshot one{net, {t1.get()}, PathWeightMode::Solo, {}, dj};
				  FleetSnapshot three{net, {t1.get(), t2.get(), t3.get()},
									  PathWeightMode::Solo, {}, dj};
				  auto alone = PrioritizedPlanner().planAll(one);
				  auto fleet = PrioritizedPlanner().planAll(three);
				  ASSERT_STR_EQ(names(IndependentPlanner().planAll(one)[0]),
								names(alone[0]), msg);
				  ASSERT_STR_EQ(names(alone[0]), names(fleet[0]), msg);
				  return true;
			  });

	suite.run("prioritized: with two lanes both trains take the short route",
			  [](std::string &msg) {
				  TrainFactory::resetIdCounter();
				  RailNetwork net;
				  for (const char *n : {"A", "B", "C", "D"})
					  net.addNode(n);
				  net.addConnection("A", "B", 10.0, 100.0, 2);
				  net.addConnection("B", "D", 10.0, 100.0, 2);
				  net.addConnection("A", "C", 10.0, 100.0);
				  net.addConnection("C", "D", 10.0, 100.0);
				  DijkstraPathfinding dj;
				  auto t1 = trainAt("T1", 28800.0);
				  auto t2 = trainAt("T2", 28800.0, "D", "A");
				  FleetSnapshot f{net, {t1.get(), t2.get()},
								  PathWeightMode::Solo, {}, dj, true};
				  auto routes = PrioritizedPlanner().planAll(f);
				  ASSERT_STR_EQ("ABD", names(routes[0]), msg);
				  ASSERT_STR_EQ("DBA", names(routes[1]), msg); // other lane
				  return true;
			  });

	suite.run("prioritized: simulated total beats independent on the diamond",
			  [](std::string &msg) {
				  std::map<std::string, std::string> routes;
				  TrainFactory::resetIdCounter();
				  std::vector<std::unique_ptr<Train>> a;
				  a.push_back(trainAt("T1", 28800.0));
				  a.push_back(trainAt("T2", 28800.0));
				  double indep = simulate(std::make_unique<IndependentPlanner>(),
										  std::move(a));
				  TrainFactory::resetIdCounter();
				  std::vector<std::unique_ptr<Train>> b;
				  b.push_back(trainAt("T1", 28800.0));
				  b.push_back(trainAt("T2", 28800.0));
				  double coop = simulate(std::make_unique<PrioritizedPlanner>(),
										 std::move(b), &routes);
				  ASSERT_STR_EQ("ABD", routes["T1"], msg);
				  ASSERT_STR_EQ("ACD", routes["T2"], msg);
				  ASSERT_TRUE(coop + 20.0 < indep, msg);
				  return true;
			  });

	/* Refined planner: the evaluate callback is the objective. */
	auto refine = [](bool withEval, double viaCWeight, std::string *out) {
		TrainFactory::resetIdCounter();
		RailNetwork net = diamond();
		DijkstraPathfinding dj;
		auto t1 = trainAD("T1");
		auto t2 = trainAD("T2");
		FleetSnapshot f{net, {t1.get(), t2.get()}, PathWeightMode::Solo, {},
						dj};
		if (withEval)
			f.evaluate = [viaCWeight](const std::vector<Route> &rs) {
				double c = 100.0;
				for (const Route &r : rs)
					if (names(r) == "ACD")
						c += viaCWeight;
				return c;
			};
		RefinedPlanner rp(std::make_unique<ForcedViaC>(),
						  std::make_unique<IndependentPlanner>());
		auto routes = rp.planAll(f);
		*out = names(routes[0]) + "," + names(routes[1]);
	};

	suite.run("refined: without evaluate it returns the inner routes",
			  [&](std::string &msg) {
				  std::string r;
				  refine(false, 0.0, &r);
				  ASSERT_STR_EQ("ACD,ACD", r, msg);
				  return true;
			  });

	suite.run("refined: a flat objective keeps the baseline routes",
			  [&](std::string &msg) {
				  std::string r;
				  refine(true, 0.0, &r);
				  ASSERT_STR_EQ("ABD,ABD", r, msg);
				  return true;
			  });

	suite.run("refined: a seed that is clearly cheaper is adopted",
			  [&](std::string &msg) {
				  std::string r;
				  refine(true, -50.0, &r);
				  ASSERT_STR_EQ("ACD,ACD", r, msg);
				  return true;
			  });

	suite.run("refined: descent moves one train when that lowers the total",
			  [&](std::string &msg) {
				  std::string r;
				  refine(true, 30.0, &r); // via C is costly: stay on B
				  ASSERT_STR_EQ("ABD,ABD", r, msg);
				  return true;
			  });

	/* CBS-lite: head-on pairs on the single-line diamond. */
	auto cbsRun = [](int budget, bool lanes, const std::string &dirs,
					 std::vector<Route> *out, CbsPlanner &cbs,
					 double *prioCost) {
		TrainFactory::resetIdCounter();
		static RailNetwork net;
		net = diamond(10.0);
		static DijkstraPathfinding dj;
		static std::vector<std::unique_ptr<Train>> ts;
		ts.clear();
		std::vector<const Train *> raw;
		for (size_t i = 0; i < dirs.size(); i++)
		{
			bool fwd = dirs[i] == 'f';
			/* the reverse train leaves one rail-time later so that it
			   meets the forward train head-on on rail B-D */
			ts.push_back(trainAt("T" + std::to_string(i + 1),
								 fwd ? 28800.0 : 29160.0,
								 fwd ? "A" : "D", fwd ? "D" : "A"));
			raw.push_back(ts.back().get());
		}
		FleetSnapshot f{net, raw, PathWeightMode::Solo, {}, dj, lanes};
		(void)budget;
		*out = cbs.planAll(f);
		if (prioCost)
			*prioCost = PrioritizedPlanner::timelineSeconds(
				f, PrioritizedPlanner().planAll(f));
		return PrioritizedPlanner::timelineSeconds(f, *out);
	};

	suite.run("cbs: no conflict equals independent with 0 expansions",
			  [&](std::string &msg) {
				  CbsPlanner cbs;
				  std::vector<Route> r;
				  cbsRun(0, false, "fr", &r, cbs, nullptr);
				  ASSERT_STR_EQ("ABD", names(r[0]), msg);
				  ASSERT_STR_EQ("DBA", names(r[1]), msg);
				  ASSERT_EQ(0, cbs.lastExpansions(), msg);
				  return true;
			  });

	suite.run("cbs: a head-on pair is split with one expansion",
			  [&](std::string &msg) {
				  CbsPlanner cbs;
				  std::vector<Route> r;
				  double prio = 0;
				  double c = cbsRun(0, true, "fr", &r, cbs, &prio);
				  ASSERT_EQ(1, cbs.lastExpansions(), msg);
				  ASSERT_TRUE(!(names(r[0]) == "ABD" && names(r[1]) == "DBA"),
							  msg);
				  ASSERT_TRUE(c <= prio, msg);
				  ASSERT_NEAR(1440.0, c, 1.0, msg);
				  return true;
			  });

	suite.run("cbs: exhausted budget keeps a valid plan and prints the notice",
			  [&](std::string &msg) {
				  CbsPlanner cbs(0);
				  std::vector<Route> r;
				  double prio = 0;
				  double c = cbsRun(0, true, "fr", &r, cbs, &prio);
				  ASSERT_STR_EQ("CBS: budget of 0 expansions reached, using "
								"the best solution found",
								cbs.lastNotice(), msg);
				  ASSERT_EQ(2u, r.size(), msg);
				  ASSERT_STR_EQ("A", r[0].front()->getName(), msg);
				  ASSERT_STR_EQ("D", r[0].back()->getName(), msg);
				  ASSERT_NEAR(prio, c, 0.001, msg);
				  return true;
			  });

	suite.run("cbs: three trains with crossing directions finish in budget",
			  [&](std::string &msg) {
				  CbsPlanner cbs;
				  std::vector<Route> r;
				  double prio = 0;
				  double c = cbsRun(0, true, "frf", &r, cbs, &prio);
				  ASSERT_EQ(3u, r.size(), msg);
				  ASSERT_TRUE(cbs.lastExpansions() <= CbsPlanner::DEFAULT_BUDGET,
							  msg);
				  ASSERT_TRUE(c <= prio, msg);
				  return true;
			  });

	/* Event re-planning: the diamond's ABD is 720 s, ACD is 756 s, so a
	   risk of R seconds on B->D gains R - 36 s for a switch. */
	auto replanVia = [](double risk, double othersExtra, bool withEval) {
		TrainFactory::resetIdCounter();
		RailNetwork net = diamond(10.5);
		DijkstraPathfinding dj;
		auto t = trainAD("T2");
		FleetSnapshot f{net, {t.get()}, PathWeightMode::Solo, {}, dj};
		if (withEval)
			f.evaluate = [othersExtra](const std::vector<Route> &rs) {
				return names(rs[0]) == "ACD" ? 100.0 + 36.0 + othersExtra
											 : 100.0;
			};
		std::vector<Route> cur = {dj.findPath("A", "D", net,
											  PathWeightMode::Solo)};
		SegmentEventRisk r{{"B->D", risk}};
		return names(ReplanPolicy::replan(f, cur, 0, r));
	};

	suite.run("replan: a gain of 29 s keeps the route, 30 s switches",
			  [&](std::string &msg) {
				  ASSERT_STR_EQ("", replanVia(65.0, 0, false), msg);
				  ASSERT_STR_EQ("ACD", replanVia(66.0, 0, false), msg);
				  return true;
			  });

	suite.run("replan: others losing more than the gain blocks the switch",
			  [&](std::string &msg) {
				  ASSERT_STR_EQ("", replanVia(96.0, 90.0, true), msg);
				  ASSERT_STR_EQ("ACD", replanVia(96.0, 50.0, true), msg);
				  return true;
			  });

	suite.run("replan: no risk means no new route (equals planAll)",
			  [&](std::string &msg) {
				  ASSERT_STR_EQ("", replanVia(0.0, 0, false), msg);
				  return true;
			  });

	return suite.summarize();
}
