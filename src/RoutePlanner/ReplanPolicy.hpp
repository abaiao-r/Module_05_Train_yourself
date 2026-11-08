#ifndef REPLANPOLICY_HPP
#define REPLANPOLICY_HPP

#include <cmath>
#include <string>
#include <vector>

#include "IRoutePlanner.hpp"
#include "SegmentTiming.hpp"

/**
 * When may a train that has not departed yet change route because of an
 * event? Only if the route is clearly better for the train itself (gain
 * of at least MIN_GAIN_SEC) AND the other trains do not lose more than
 * that gain. The threshold stops flip-flopping on tiny differences.
 */
struct ReplanPolicy
{
	static constexpr double MIN_GAIN_SEC = 30.0;

	/* Free-flow seconds of the route + risk seconds per segment + the stop
	   at every intermediate station. */
	static double routeSeconds(const RailNetwork &net, const Route &r,
							   const SegmentEventRisk &risk, double dwell)
	{
		double total = 0.0;
		for (size_t k = 0; k + 1 < r.size(); k++)
		{
			const std::string &from = r[k]->getName();
			const std::string &to = r[k + 1]->getName();
			for (const Edge &e : net.getNeighbours(from))
				if (e.destination->getName() == to)
					total += freeFlowSeconds(e);
			auto it = risk.find(from + "->" + to);
			if (it != risk.end())
				total += it->second;
			if (k + 2 < r.size())
				total += dwell;
		}
		return total;
	}

	static bool worthSwitching(double ownGain, double othersExtra)
	{
		return ownGain >= MIN_GAIN_SEC && othersExtra <= ownGain;
	}

	/* Better route for train idx, or an empty route to keep the current
	   one. `routes` holds every train's current route. */
	static Route replan(const FleetSnapshot &fleet,
						const std::vector<Route> &routes, size_t idx,
						const SegmentEventRisk &risk)
	{
		const Train &t = *fleet.trains[idx];
		Route cand;
		try
		{
			cand = fleet.pathfinder.findConstrainedPath(
				t.getDepartureStation(), t.getArrivalStation(), fleet.network,
				{}, risk, t.getStopDuration());
		}
		catch (const std::exception &)
		{
			return {};
		}
		if (cand.empty() || sameRoute(cand, routes[idx]))
			return {};

		double dwell = t.getStopDuration();
		double gain = routeSeconds(fleet.network, routes[idx], risk, dwell)
					  - routeSeconds(fleet.network, cand, risk, dwell);
		double othersExtra = 0.0;
		if (fleet.evaluate)
		{
			std::vector<Route> trial = routes;
			trial[idx] = cand;
			double before = fleet.evaluate(routes);
			double after = fleet.evaluate(trial);
			if (!std::isfinite(before) || !std::isfinite(after))
				return {};
			double ownFreeFlow =
				routeSeconds(fleet.network, cand, {}, dwell)
				- routeSeconds(fleet.network, routes[idx], {}, dwell);
			othersExtra = (after - before) - ownFreeFlow;
		}
		return worthSwitching(gain, othersExtra) ? cand : Route{};
	}

  private:
	static bool sameRoute(const Route &a, const Route &b)
	{
		if (a.size() != b.size())
			return false;
		for (size_t i = 0; i < a.size(); i++)
			if (a[i]->getName() != b[i]->getName())
				return false;
		return true;
	}
};

#endif
