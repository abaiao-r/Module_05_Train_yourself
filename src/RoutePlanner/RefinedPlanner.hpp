#ifndef REFINEDPLANNER_HPP
#define REFINEDPLANNER_HPP

#include <algorithm>
#include <limits>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <utility>

#include "IRoutePlanner.hpp"

/**
 * Decorator: improves the routes of another planner by coordinate descent
 * on the REAL fleet objective. Starting from the best of {inner planner,
 * independent routes}, it tries to move one train at a time onto an
 * alternative route and keeps the move only if the dry-run total travel
 * time of the whole fleet drops by at least MIN_GAIN_SEC. Because every
 * accepted move lowers that total, the result is never worse than either
 * seed in the dry run, and a train may now take a slower route to spare
 * the others (the selfish-routing fix).
 *
 * Without FleetSnapshot::evaluate it returns the inner planner's routes.
 */
class RefinedPlanner : public IRoutePlanner
{
  public:
	static constexpr double MIN_GAIN_SEC = 1.0;
	static constexpr int MAX_PASSES = 3;

	RefinedPlanner(std::unique_ptr<IRoutePlanner> inner,
				   std::unique_ptr<IRoutePlanner> baseline)
		: _inner(std::move(inner)), _baseline(std::move(baseline)) {}

	std::vector<Route> planAll(const FleetSnapshot &fleet) override
	{
		if (!fleet.evaluate)
			return _inner->planAll(fleet);

		/* The baseline wins ties: only a real gain justifies moving away
		   from the route every train would pick on its own. */
		std::vector<Route> best = _baseline->planAll(fleet);
		double bestCost = cost(fleet, best);
		std::vector<Route> seed = _inner->planAll(fleet);
		double seedCost = cost(fleet, seed);
		if (seedCost < bestCost - MIN_GAIN_SEC)
		{
			best = seed;
			bestCost = seedCost;
		}

		std::vector<size_t> order(fleet.trains.size());
		std::iota(order.begin(), order.end(), 0);
		std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) {
			const Train &x = *fleet.trains[a];
			const Train &y = *fleet.trains[b];
			if (x.getDepartureTime() != y.getDepartureTime())
				return x.getDepartureTime() < y.getDepartureTime();
			return x.getName() < y.getName();
		});

		for (int pass = 0; pass < MAX_PASSES; pass++)
		{
			bool improved = false;
			for (size_t i : order)
			{
				for (const Route &cand : alternatives(fleet, i, best[i]))
				{
					std::vector<Route> trial = best;
					trial[i] = cand;
					double c = cost(fleet, trial);
					if (c < bestCost - MIN_GAIN_SEC)
					{
						best = trial;
						bestCost = c;
						improved = true;
					}
				}
			}
			if (!improved)
				break;
		}
		return best;
	}

	const char *name() const override { return "cooperative"; }

  private:
	/* Dry-run total plus the expected random-event delay on every route:
	   the dry run is event-free, so without this term a plan could trade
	   a little traffic for a lot of event exposure. */
	static double cost(const FleetSnapshot &fleet,
					   const std::vector<Route> &routes)
	{
		double total = fleet.evaluate(routes);
		for (const Route &r : routes)
			for (size_t k = 0; k + 1 < r.size(); k++)
			{
				auto it = fleet.eventRisk.find(r[k]->getName() + "->"
											   + r[k + 1]->getName());
				if (it != fleet.eventRisk.end())
					total += it->second;
			}
		return total;
	}

	static std::string routeKey(const Route &r)
	{
		std::string k;
		for (const auto &n : r)
			k += n->getName() + ">";
		return k;
	}

	/* Routes that avoid, in turn, each rail of the current one. */
	static std::vector<Route> alternatives(const FleetSnapshot &fleet,
										   size_t idx, const Route &current)
	{
		const Train &t = *fleet.trains[idx];
		std::set<std::string> seen{routeKey(current)};
		std::vector<Route> out;
		for (size_t k = 0; k + 1 < current.size(); k++)
		{
			const std::string &a = current[k]->getName();
			const std::string &b = current[k + 1]->getName();
			ForbiddenEdges forbidden{a + "->" + b, b + "->" + a};
			Route r = fleet.pathfinder.findConstrainedPath(
				t.getDepartureStation(), t.getArrivalStation(), fleet.network,
				forbidden, fleet.eventRisk, t.getStopDuration());
			if (!r.empty() && seen.insert(routeKey(r)).second)
				out.push_back(r);
		}
		return out;
	}

	std::unique_ptr<IRoutePlanner> _inner;
	std::unique_ptr<IRoutePlanner> _baseline;
};

#endif
