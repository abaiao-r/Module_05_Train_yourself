#ifndef CBSPLANNER_HPP
#define CBSPLANNER_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "IRoutePlanner.hpp"
#include "IndependentPlanner.hpp"
#include "PrioritizedPlanner.hpp"
#include "SegmentTiming.hpp"

/**
 * Conflict-Based Search, lite. Every train starts on its own route. The
 * first head-on conflict (two trains on a single-line rail in opposite
 * directions in overlapping free-flow windows) is resolved by branching:
 * one branch forbids that rail to the first train, the other to the
 * second (lower name first), and only that train is re-planned. Nodes are
 * expanded best-first by fleet cost. The prioritized solution is the
 * upper bound, so the result is never costlier than PrioritizedPlanner.
 *
 * Cost: event-free FleetSnapshot::evaluate if provided, else the
 * prioritized free-flow timeline, plus expected event delay on each route.
 * Conflicts exist only when lanes are enabled.
 */
class CbsPlanner : public IRoutePlanner
{
  public:
	static constexpr int DEFAULT_BUDGET = 200;

	explicit CbsPlanner(int budget = DEFAULT_BUDGET) : _budget(budget) {}

	std::vector<Route> planAll(const FleetSnapshot &fleet) override
	{
		_expansions = 0;
		_notice.clear();

		std::vector<Route> best = PrioritizedPlanner().planAll(fleet);
		double bestCost = cost(fleet, best);

		Node root;
		root.forbidden.assign(fleet.trains.size(), {});
		root.routes = IndependentPlanner().planAll(fleet);
		root.cost = cost(fleet, root.routes);

		auto worse = [](const Node &a, const Node &b) {
			return a.cost > b.cost;
		};
		std::priority_queue<Node, std::vector<Node>, decltype(worse)> open(
			worse);
		std::set<std::string> seen{signature(root)};
		open.push(root);

		while (!open.empty())
		{
			Node node = open.top();
			open.pop();
			size_t a = 0, b = 0;
			std::string rail;
			if (!firstConflict(fleet, node.routes, a, b, rail))
			{
				if (node.cost < bestCost)
				{
					best = node.routes;
					bestCost = node.cost;
				}
				break; // best-first: the first conflict-free node is optimal
			}
			if (_expansions >= _budget)
			{
				_notice = "CBS: budget of " + std::to_string(_budget)
						  + " expansions reached, using the best solution "
							"found";
				std::cout << _notice << std::endl;
				break;
			}
			_expansions++;
			for (size_t who : {a, b})
			{
				Node child = node;
				child.forbidden[who].insert(rail);
				if (!seen.insert(signature(child)).second)
					continue;
				if (!replan(fleet, child, who))
					continue;
				child.cost = cost(fleet, child.routes);
				open.push(child);
			}
		}
		return best;
	}

	const char *name() const override { return "cbs"; }

	int lastExpansions() const { return _expansions; }
	const std::string &lastNotice() const { return _notice; }

  private:
	struct Node
	{
		std::vector<Route> routes;
		std::vector<std::set<std::string>> forbidden; // "a|b" rails per train
		double cost = 0.0;
	};

	static std::string railKey(const std::string &x, const std::string &y)
	{
		return x < y ? x + "|" + y : y + "|" + x;
	}

	static std::string signature(const Node &n)
	{
		std::string s;
		for (const auto &f : n.forbidden)
		{
			for (const auto &r : f)
				s += r + ",";
			s += ";";
		}
		return s;
	}

	static double cost(const FleetSnapshot &fleet,
					   const std::vector<Route> &routes)
	{
		double total = fleet.evaluate
						   ? fleet.evaluate(routes)
						   : PrioritizedPlanner::timelineSeconds(fleet, routes);
		for (const Route &route : routes)
			for (size_t k = 0; k + 1 < route.size(); ++k)
			{
				const std::string key = route[k]->getName() + "->"
										+ route[k + 1]->getName();
				auto risk = fleet.eventRisk.find(key);
				if (risk != fleet.eventRisk.end())
					total += risk->second;
			}
		return total;
	}

	static bool replan(const FleetSnapshot &fleet, Node &node, size_t who)
	{
		const Train &t = *fleet.trains[who];
		ForbiddenEdges forbidden;
		for (const std::string &r : node.forbidden[who])
		{
			size_t bar = r.find('|');
			std::string x = r.substr(0, bar), y = r.substr(bar + 1);
			forbidden.insert(x + "->" + y);
			forbidden.insert(y + "->" + x);
		}
		try
		{
			Route r = fleet.pathfinder.findConstrainedPath(
				t.getDepartureStation(), t.getArrivalStation(), fleet.network,
				forbidden, fleet.eventRisk, t.getStopDuration());
			if (r.empty())
				return false;
			node.routes[who] = r;
			return true;
		}
		catch (const std::exception &)
		{
			return false;
		}
	}

	struct Window
	{
		size_t train;
		bool forward; // name order of the endpoints
		double start, end;
	};

	static int railLines(const FleetSnapshot &fleet, const std::string &from,
						 const std::string &to)
	{
		for (const Edge &e : fleet.network.getNeighbours(from))
			if (e.destination->getName() == to)
				return e.numLines;
		return 1;
	}

	/* First (by time, then train names) head-on conflict, if any. */
	static bool firstConflict(const FleetSnapshot &fleet,
							  const std::vector<Route> &routes, size_t &a,
							  size_t &b, std::string &rail)
	{
		if (!fleet.lanesEnabled)
			return false;
		std::map<std::string, std::vector<Window>> byRail;
		for (size_t i = 0; i < routes.size(); i++)
		{
			const Train &t = *fleet.trains[i];
			double clock = t.getDepartureTime();
			const Route &r = routes[i];
			for (size_t k = 0; k + 1 < r.size(); k++)
			{
				const std::string &from = r[k]->getName();
				const std::string &to = r[k + 1]->getName();
				double dur = 0.0;
				for (const Edge &e : fleet.network.getNeighbours(from))
					if (e.destination->getName() == to)
						dur = freeFlowSeconds(e, 0);
				byRail[railKey(from, to)].push_back(
					{i, from < to, clock, clock + dur});
				clock += dur + t.getStopDuration();
			}
		}
		bool found = false;
		double bestT = 0.0;
		for (const auto &entry : byRail)
		{
			size_t bar = entry.first.find('|');
			if (railLines(fleet, entry.first.substr(0, bar),
						  entry.first.substr(bar + 1))
				> 1)
				continue;
			const auto &w = entry.second;
			for (size_t i = 0; i < w.size(); i++)
				for (size_t j = i + 1; j < w.size(); j++)
				{
					if (w[i].forward == w[j].forward)
						continue;
					double s = std::max(w[i].start, w[j].start);
					if (s >= std::min(w[i].end, w[j].end))
						continue;
					size_t x = w[i].train, y = w[j].train;
					if (fleet.trains[y]->getName() < fleet.trains[x]->getName())
						std::swap(x, y);
					if (!found || s < bestT)
					{
						found = true;
						bestT = s;
						a = x;
						b = y;
						rail = entry.first;
					}
				}
		}
		return found;
	}

	int _budget;
	int _expansions = 0;
	std::string _notice;
};

#endif
