/* Bonus: multiple lines per edge, head-on exclusion (--lines). */

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "DijkstraPathfinding.hpp"
#include "Simulation.hpp"
#include "TrainFactory.hpp"
#include "RailNetwork.hpp"
#include "TestFramework.hpp"

struct Outcome
{
	std::map<std::string, double> actual;
	std::map<std::string, std::vector<int>> linesSeen;
	bool headOn = false;
	bool legacyOverlap = false;
};

/* Runs the trains in lines mode and records, every tick, whether two
   opposite-direction trains ever shared a line. */
static Outcome runLines(RailNetwork net,
						const std::vector<std::tuple<std::string, std::string,
													 std::string, double>> &spec,
						bool enabled = true,
						PathWeightMode mode = PathWeightMode::Distance)
{
	TrainFactory::resetIdCounter();
	std::vector<std::unique_ptr<Train>> trains;
	for (const auto &s : spec)
		trains.push_back(TrainFactory::createTrain(
			std::get<0>(s), 80.0, 0.05, 356.0, 30.0, std::get<1>(s),
			std::get<2>(s), std::get<3>(s), 0.0));
	LinesOptions lo;
	lo.enabled = enabled;
	Simulation sim(std::move(net), std::move(trains), {},
				   std::make_unique<DijkstraPathfinding>(),
				   mode, lo);
	sim.setQuiet(true);
	Outcome out;
	sim.setAnimCallback([&](double, const std::vector<TrainState> &st) {
		for (size_t i = 0; i < st.size(); i++)
		{
			const auto &a = st[i];
			if (!a.departed || a.arrived || a.waiting || a.lineId < 0)
				continue;
			const auto &pa = a.train->getPath();
			auto &seen = out.linesSeen[a.train->getName()];
			if (seen.empty() || seen.back() != a.lineId)
				seen.push_back(a.lineId);
			for (size_t j = i + 1; j < st.size(); j++)
			{
				const auto &b = st[j];
				if (!b.departed || b.arrived || b.waiting
					|| b.lineId != a.lineId)
					continue;
				const auto &pb = b.train->getPath();
				if (pa[a.segmentIndex] == pb[b.segmentIndex + 1]
					&& pa[a.segmentIndex + 1] == pb[b.segmentIndex])
					out.headOn = true;
			}
		}
	});
	sim.run();
	for (const auto &r : sim.getResults())
		out.actual[r.name] = r.actualTime;
	return out;
}

static RailNetwork twoNodesSpeeds(const std::vector<double> &speeds)
{
	RailNetwork net;
	net.addNode("A");
	net.addNode("B");
	net.addConnection("A", "B", 10.0, 100.0, static_cast<int>(speeds.size()),
					  speeds);
	return net;
}

static RailNetwork twoNodes(int lines, double km = 10.0)
{
	RailNetwork net;
	net.addNode("A");
	net.addNode("B");
	net.addConnection("A", "B", km, 100.0, lines);
	return net;
}

int main()
{
	Test::TestSuite suite("Lines");

	suite.run("one line: opposite trains are serialised, never head-on",
			  [](std::string &msg) {
				  auto o = runLines(twoNodes(1), {{"TA", "A", "B", 100.0},
												  {"TB", "B", "A", 100.0}});
				  ASSERT_FALSE(o.headOn, msg);
				  /* TA wins the tie by name; TB runs only after TA clears,
				     so it takes about twice as long. */
				  double ta = o.actual["TA"];
				  ASSERT_NEAR(2.0 * ta, o.actual["TB"], 3.0, msg);
				  return true;
			  });

	suite.run("two lines: opposite trains run at once on different lines",
			  [](std::string &msg) {
				  auto o = runLines(twoNodes(2), {{"TA", "A", "B", 100.0},
												  {"TB", "B", "A", 100.0}});
				  ASSERT_FALSE(o.headOn, msg);
				  ASSERT_NEAR(o.actual["TA"], o.actual["TB"], 1.0, msg);
				  ASSERT_EQ(0, o.linesSeen["TA"].front(), msg);
				  ASSERT_EQ(1, o.linesSeen["TB"].front(), msg);
				  return true;
			  });

	suite.run("same direction on one line shares the line",
			  [](std::string &msg) {
				  auto o = runLines(twoNodes(1), {{"T1", "A", "B", 100.0},
												  {"T2", "A", "B", 110.0}});
				  ASSERT_EQ(1, static_cast<int>(o.linesSeen["T1"].size()),
							msg);
				  ASSERT_EQ(0, o.linesSeen["T2"].front(), msg);
				  /* T2 follows T1 (about 10 s behind), it does not wait for
				     the line to clear. */
				  ASSERT_NEAR(o.actual["T1"] + 10.0, o.actual["T2"], 15.0,
							  msg);
				  return true;
			  });

	suite.run("same direction picks the free line first",
			  [](std::string &msg) {
				  auto o = runLines(twoNodes(2), {{"T1", "A", "B", 100.0},
												  {"T2", "A", "B", 110.0}});
				  ASSERT_EQ(0, o.linesSeen["T1"].front(), msg);
				  ASSERT_EQ(1, o.linesSeen["T2"].front(), msg);
				  return true;
			  });

	suite.run("disabled mode keeps legacy behaviour (no waiting)",
			  [](std::string &msg) {
				  auto o = runLines(twoNodes(1), {{"TA", "A", "B", 100.0},
												  {"TB", "B", "A", 100.0}},
									false);
				  ASSERT_NEAR(o.actual["TA"], o.actual["TB"], 1.0, msg);
				  ASSERT_TRUE(o.linesSeen.empty(), msg);
				  return true;
			  });

	suite.run("opposite trains meeting mid-route on a path wait at node",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("A");
				  net.addNode("B");
				  net.addNode("C");
				  net.addConnection("A", "B", 10.0, 100.0, 1);
				  net.addConnection("B", "C", 10.0, 100.0, 1);
				  auto o = runLines(net, {{"TA", "A", "C", 100.0},
										  {"TC", "C", "A", 100.0}});
				  ASSERT_FALSE(o.headOn, msg);
				  ASSERT_TRUE(o.actual["TA"] > 0.0, msg);
				  ASSERT_TRUE(o.actual["TC"] > 0.0, msg);
				  return true;
			  });

	suite.run("fast train overtakes a slow one via the second line",
			  [](std::string &msg) {
				  auto run = [](int lines) {
					  TrainFactory::resetIdCounter();
					  std::vector<std::unique_ptr<Train>> t;
					  t.push_back(TrainFactory::createTrain(
						  "Slow", 400.0, 0.05, 220.0, 30.0, "A", "B", 100.0,
						  0.0));
					  t.push_back(TrainFactory::createTrain(
						  "Fast", 80.0, 0.05, 356.0, 30.0, "A", "B", 160.0,
						  0.0));
					  Simulation sim(twoNodes(lines, 30.0), std::move(t), {},
									 std::make_unique<DijkstraPathfinding>(),
									 PathWeightMode::Distance,
									 LinesOptions{true});
					  sim.setQuiet(true);
					  sim.run();
					  std::map<std::string, double> arr;
					  for (const auto &r : sim.getResults())
						  arr[r.name] = r.actualTime
										+ (r.name == "Slow" ? 100.0 : 160.0);
					  return arr;
				  };
				  auto one = run(1);
				  auto two = run(2);
				  ASSERT_TRUE(one["Fast"] > one["Slow"], msg);
				  ASSERT_TRUE(two["Fast"] < two["Slow"], msg);
				  return true;
			  });

	suite.run("dijkstra: a spare line keeps the edge free of congestion",
			  [](std::string &msg) {
				  auto build = [](int lines) {
					  RailNetwork net;
					  for (const char *n : {"A", "B", "C", "D"})
						  net.addNode(n);
					  net.addConnection("A", "B", 10.0, 100.0, lines);
					  net.addConnection("B", "D", 10.0, 100.0);
					  net.addConnection("A", "C", 10.0, 100.0);
					  net.addConnection("C", "D", 10.0, 100.0);
					  return net;
				  };
				  DijkstraPathfinding dj;
				  std::string base = dj.findPath("A", "D", build(2),
									PathWeightMode::Solo)[1]->getName();
				  SegmentOccupancy occ;
				  occ["A->" + base] = 1;
				  auto net2 = build(2);
				  auto net1 = build(1);
				  /* base route is the 2-line edge only if base == "B" */
				  ASSERT_STR_EQ("B", base, msg);
				  ASSERT_STR_EQ("B", dj.findPath("A", "D", net2,
							PathWeightMode::Adaptive, occ)[1]->getName(), msg);
				  ASSERT_STR_EQ("C", dj.findPath("A", "D", net1,
							PathWeightMode::Adaptive, occ)[1]->getName(), msg);
				  occ["A->B"] = 2;
				  ASSERT_STR_EQ("C", dj.findPath("A", "D", net2,
							PathWeightMode::Adaptive, occ)[1]->getName(), msg);
				  return true;
			  });

	suite.run("a stream of same-direction trains cannot starve an opposite one",
			  [](std::string &msg) {
				  auto o = runLines(twoNodes(1), {{"T1", "A", "B", 100.0},
												  {"R", "B", "A", 105.0},
												  {"T2", "A", "B", 110.0},
												  {"T3", "A", "B", 120.0}});
				  ASSERT_FALSE(o.headOn, msg);
				  /* R queued before T2 and T3, so it crosses right after
				     T1 and both later trains wait behind it. */
				  ASSERT_TRUE(105.0 + o.actual["R"] < 110.0 + o.actual["T2"],
							  msg);
				  ASSERT_TRUE(105.0 + o.actual["R"] < 120.0 + o.actual["T3"],
							  msg);
				  ASSERT_TRUE(100.0 + o.actual["T1"] < 105.0 + o.actual["R"],
							  msg);
				  return true;
			  });

	suite.run("trains circulating a triangle of single lines all arrive",
			  [](std::string &msg) {
				  RailNetwork net;
				  for (const char *n : {"A", "B", "C"})
					  net.addNode(n);
				  net.addConnection("A", "B", 10.0, 100.0);
				  net.addConnection("B", "C", 10.0, 100.0);
				  net.addConnection("A", "C", 10.0, 100.0);
				  auto o = runLines(net, {{"T1", "A", "B", 100.0},
										  {"T2", "B", "C", 100.0},
										  {"T3", "C", "A", 100.0},
										  {"T4", "B", "A", 100.0},
										  {"T5", "C", "B", 100.0},
										  {"T6", "A", "C", 100.0}});
				  ASSERT_FALSE(o.headOn, msg);
				  ASSERT_EQ(6, static_cast<int>(o.actual.size()), msg);
				  for (const auto &kv : o.actual)
					  ASSERT_TRUE(kv.second > 0.0, msg);
				  return true;
			  });

	suite.run("swap through a shared middle station never deadlocks",
			  [](std::string &msg) {
				  RailNetwork net;
				  for (const char *n : {"A", "B", "C"})
					  net.addNode(n);
				  net.addConnection("A", "B", 10.0, 100.0);
				  net.addConnection("B", "C", 10.0, 100.0);
				  auto o = runLines(net, {{"TA", "A", "C", 100.0},
										  {"TC", "C", "A", 100.0},
										  {"TD", "A", "C", 100.0},
										  {"TE", "C", "A", 100.0}});
				  ASSERT_FALSE(o.headOn, msg);
				  ASSERT_EQ(4, static_cast<int>(o.actual.size()), msg);
				  return true;
			  });

	suite.run("live AddRail carries numLines only when lines mode is on",
			  [](std::string &msg) {
				  auto count = [](bool enabled) {
					  TrainFactory::resetIdCounter();
					  RailNetwork net = twoNodes(1);
					  net.addNode("C");
					  std::vector<std::unique_ptr<Train>> t;
					  t.push_back(TrainFactory::createTrain(
						  "T", 80.0, 0.05, 356.0, 30.0, "A", "B", 100.0,
						  0.0));
					  Simulation sim(std::move(net), std::move(t), {},
									 std::make_unique<DijkstraPathfinding>(),
									 PathWeightMode::Distance,
									 LinesOptions{enabled});
					  sim.setQuiet(true);
					  bool done = false;
					  sim.setAnimCallback(
						  [&](double, const std::vector<TrainState> &) {
							  if (!done)
								  sim.enqueueMutation(AddRailCommand{
									  "B", "C", 5.0, 100.0, 3});
							  done = true;
						  });
					  sim.run();
					  for (const auto &e : sim.getNetwork().getNeighbours("B"))
						  if (e.destination->getName() == "C")
							  return e.numLines;
					  return -1;
				  };
				  ASSERT_EQ(3, count(true), msg);
				  ASSERT_EQ(1, count(false), msg);
				  return true;
			  });

	suite.run("per-line speed: first train takes the fast line, second the slow",
			  [](std::string &msg) {
				  auto o = runLines(twoNodesSpeeds({50.0, 100.0}),
									{{"T1", "A", "B", 100.0},
									 {"T2", "A", "B", 100.0}});
				  auto ref = runLines(twoNodes(1), {{"R", "A", "B", 100.0}});
				  ASSERT_EQ(1, o.linesSeen["T1"].front(), msg);
				  ASSERT_EQ(0, o.linesSeen["T2"].front(), msg);
				  ASSERT_NEAR(ref.actual["R"], o.actual["T1"], 1.5, msg);
				  ASSERT_TRUE(o.actual["T2"] > 1.6 * o.actual["T1"], msg);
				  return true;
			  });

	suite.run("adaptive: a train blocked by opposite traffic detours",
			  [](std::string &msg) {
				  auto mk = [] {
					  RailNetwork net;
					  net.addNode("A");
					  net.addNode("B");
					  net.addNode("C");
					  net.addConnection("A", "B", 10.0, 100.0, 1);
					  net.addConnection("A", "C", 6.0, 100.0, 1);
					  net.addConnection("C", "B", 6.0, 100.0, 1);
					  return net;
				  };
				  std::vector<std::tuple<std::string, std::string,
										 std::string, double>> spec
					  = {{"TA", "A", "B", 100.0}, {"TB", "B", "A", 100.0}};
				  auto stay = runLines(mk(), spec, true,
									   PathWeightMode::Distance);
				  auto det = runLines(mk(), spec, true,
									  PathWeightMode::Adaptive);
				  ASSERT_FALSE(det.headOn, msg);
				  ASSERT_NEAR(2.0 * stay.actual["TA"], stay.actual["TB"],
							  3.0, msg);
				  ASSERT_TRUE(det.actual["TB"] + 30.0 < stay.actual["TB"],
							  msg);
				  return true;
			  });

	return suite.summarize();
}
