#ifndef PRIORITIZEDPLANNER_HPP
#define PRIORITIZEDPLANNER_HPP

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <string>

#include "CooperativeCost.hpp"
#include "IRoutePlanner.hpp"

/**
 * Prioritized planning: trains are routed one at a time in a fixed
 * priority order (earliest departure first, ties by train name). Each
 * train is routed against the reservations of the trains before it, then
 * its route is committed and reserved. A committed route is never touched
 * again, so there is no reroute cascade.
 */
class PrioritizedPlanner : public IRoutePlanner
{
  public:
	std::vector<Route> planAll(const FleetSnapshot &fleet) override
	{
		std::vector<size_t> order(fleet.trains.size());
		std::iota(order.begin(), order.end(), 0);
		std::stable_sort(order.begin(), order.end(),
						 [&](size_t a, size_t b) {
							 const Train &x = *fleet.trains[a];
							 const Train &y = *fleet.trains[b];
							 if (x.getDepartureTime() != y.getDepartureTime())
								 return x.getDepartureTime()
										< y.getDepartureTime();
							 return x.getName() < y.getName();
						 });

		ReservationTable table(fleet.lanesEnabled);
		std::vector<Route> routes(fleet.trains.size());
		for (size_t i : order)
		{
			routes[i] = planOne(fleet, table, *fleet.trains[i]);
			reserve(fleet, table, routes[i], *fleet.trains[i]);
		}
		return routes;
	}

	const char *name() const override { return "cooperative"; }

  private:
	static Route planOne(const FleetSnapshot &fleet,
						 const ReservationTable &table, const Train &train)
	{
		double dwell = train.getStopDuration();
		SegmentEventRisk risk =
			tollMap(table, fleet.network, train.getDepartureStation(),
					train.getDepartureTime(), dwell, fleet.lanesEnabled,
					fleet.eventRisk);
		Route path;
		try
		{
			path = fleet.pathfinder.findConstrainedPath(
				train.getDepartureStation(), train.getArrivalStation(),
				fleet.network, {}, risk, dwell);
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("No path found for " + train.getName()
									 + ": " + e.what());
		}
		if (path.empty())
			throw std::runtime_error(
				"No path found for " + train.getName() + " ("
				+ train.getDepartureStation() + " -> "
				+ train.getArrivalStation() + ")");
		return path;
	}

	/* Walks the route on a free-flow timeline (waiting for opposite
	   traffic where needed) and books every segment. */
	static void reserve(const FleetSnapshot &fleet, ReservationTable &table,
						const Route &route, const Train &train)
	{
		double t = train.getDepartureTime();
		for (size_t k = 0; k + 1 < route.size(); k++)
		{
			const std::string &from = route[k]->getName();
			const std::string &to = route[k + 1]->getName();
			for (const Edge &e : fleet.network.getNeighbours(from))
			{
				if (e.destination->getName() != to)
					continue;
				int lane = bestLane(table, from, e, t, fleet.lanesEnabled).first;
				double dur = freeFlowSeconds(e, lane);
				double start = table.earliestFree(from, to, lane, t, dur);
				table.reserve(from, to, lane, start, start + dur);
				t = start + dur;
				if (k + 2 < route.size())
					t += train.getStopDuration();
				break;
			}
		}
	}
};

#endif
