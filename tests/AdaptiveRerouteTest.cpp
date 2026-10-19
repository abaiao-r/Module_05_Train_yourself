/* Adaptive reroute: "stuck behind a slower train, does a detour pay?" */

#include <memory>
#include <string>
#include <vector>

#include "DijkstraPathfinding.hpp"
#include "InputHandler.hpp"
#include "Simulation.hpp"
#include "TestFramework.hpp"

static const std::string NET = "input/railNetworkPrintFolder/railNetworkScenario";
static const std::string TRN = "input/trainPrintFolder/trainPrintScenario";

struct Outcome
{
	std::vector<std::string> rerouted;
	std::vector<TrainResult> results;
};

class CountingPathfinder : public DijkstraPathfinding
{
  public:
	static int constrainedCalls;
	std::vector<std::shared_ptr<Node>> findConstrainedPath(
		const std::string &s, const std::string &e, const RailNetwork &n,
		const ForbiddenEdges &f, const SegmentEventRisk &r,
		double dwell) const override
	{
		constrainedCalls++;
		return DijkstraPathfinding::findConstrainedPath(s, e, n, f, r, dwell);
	}
};
int CountingPathfinder::constrainedCalls = 0;

static Outcome run(const std::string &name, PathWeightMode mode)
{
	std::string base = name;
	auto data = InputHandler::loadData(NET + base + ".txt", TRN + base + ".txt");
	Simulation sim(std::move(data.network), std::move(data.trains),
				   std::move(data.events),
				   std::make_unique<CountingPathfinder>(), mode);
	sim.setQuiet(true);
	Outcome o;
	sim.setRerouteCallback([&](const std::string &t, const std::string &,
							   const std::string &) { o.rerouted.push_back(t); });
	sim.run();
	o.results = sim.getResults();
	return o;
}

static bool rerouted(const Outcome &o, const std::string &t)
{
	for (const auto &r : o.rerouted)
		if (r == t)
			return true;
	return false;
}

static double actual(const Outcome &o, const std::string &t)
{
	for (const auto &r : o.results)
		if (r.name == t)
			return r.actualTime;
	return -1;
}

int main()
{
	Test::TestSuite suite("AdaptiveReroute");

	suite.run("reroute pays: fast train detours and arrives sooner",
			  [](std::string &msg) {
		Outcome a = run("2BlockReroutePays", PathWeightMode::Adaptive);
		Outcome s = run("2BlockReroutePays", PathWeightMode::Solo);
		ASSERT_TRUE(rerouted(a, "FastB"), msg);
		ASSERT_FALSE(rerouted(a, "SlowA"), msg);
		ASSERT_TRUE(actual(a, "FastB") + 30 < actual(s, "FastB"), msg);
		return true;
	});

	suite.run("stay is better: long detour is rejected", [](std::string &msg) {
		Outcome a = run("3BlockStayBetter", PathWeightMode::Adaptive);
		ASSERT_TRUE(a.rerouted.empty(), msg);
		return true;
	});

	suite.run("leader diverges: no detour search at all", [](std::string &msg) {
		CountingPathfinder::constrainedCalls = 0;
		Outcome a = run("7LeaderDiverges", PathWeightMode::Adaptive);
		ASSERT_TRUE(a.rerouted.empty(), msg);
		ASSERT_EQ(0, CountingPathfinder::constrainedCalls, msg);
		return true;
	});

	suite.run("multi-train: only the first fast train reroutes",
			  [](std::string &msg) {
		Outcome a = run("4MultiTrainCongestion", PathWeightMode::Adaptive);
		ASSERT_TRUE(rerouted(a, "FastB"), msg);
		ASSERT_FALSE(rerouted(a, "SlowA"), msg);
		ASSERT_TRUE(a.rerouted.size() <= 3, msg);
		return true;
	});

	suite.run("detour searches are bounded", [](std::string &msg) {
		CountingPathfinder::constrainedCalls = 0;
		run("4MultiTrainCongestion", PathWeightMode::Adaptive);
		/* 3 followers x at most 3 evaluations per segment x few segments */
		ASSERT_TRUE(CountingPathfinder::constrainedCalls <= 30, msg);
		return true;
	});

	suite.run("diverging nearest leader is skipped for one further ahead",
			  [](std::string &msg) {
		Outcome a = run("8SkipDivergingLeader", PathWeightMode::Adaptive);
		ASSERT_TRUE(rerouted(a, "FastC"), msg);
		ASSERT_FALSE(rerouted(a, "DivB"), msg);
		return true;
	});

	suite.run("non-adaptive modes never reroute", [](std::string &msg) {
		Outcome a = run("2BlockReroutePays", PathWeightMode::Solo);
		ASSERT_TRUE(a.rerouted.empty(), msg);
		return true;
	});

	suite.run("deterministic: same input gives same result",
			  [](std::string &msg) {
		Outcome a = run("4MultiTrainCongestion", PathWeightMode::Adaptive);
		Outcome b = run("4MultiTrainCongestion", PathWeightMode::Adaptive);
		for (const auto &r : a.results)
			ASSERT_NEAR(r.actualTime, actual(b, r.name), 1e-9, msg);
		return true;
	});

	suite.run("constrained path avoids forbidden edge and ends empty if cut",
			  [](std::string &msg) {
		auto data = InputHandler::loadData(NET + "2BlockReroutePays.txt",
										   TRN + "2BlockReroutePays.txt");
		DijkstraPathfinding d;
		ForbiddenEdges f = {"Fork->MainA"};
		auto p = d.findConstrainedPath("Fork", "Dest", data.network, f);
		ASSERT_EQ(3, (int)p.size(), msg);
		ASSERT_STR_EQ("AltA", p[1]->getName(), msg);
		f.insert("Fork->AltA");
		ASSERT_TRUE(d.findConstrainedPath("Fork", "Dest", data.network, f)
						.empty(), msg);
		return true;
	});

	return suite.summarize();
}
