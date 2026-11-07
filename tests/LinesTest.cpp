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
						bool enabled = true)
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
				   PathWeightMode::Distance, lo);
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

static RailNetwork twoNodes(int lines)
{
	RailNetwork net;
	net.addNode("A");
	net.addNode("B");
	net.addConnection("A", "B", 10.0, 100.0, lines);
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

	return suite.summarize();
}
