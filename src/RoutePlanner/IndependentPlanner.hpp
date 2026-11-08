#ifndef INDEPENDENTPLANNER_HPP
#define INDEPENDENTPLANNER_HPP

#include <stdexcept>
#include <string>

#include "IRoutePlanner.hpp"

/**
 * The original behaviour: every train asks the pathfinder for its own
 * best route, ignoring all other trains.
 */
class IndependentPlanner : public IRoutePlanner
{
  public:
	std::vector<Route> planAll(const FleetSnapshot &fleet) override
	{
		std::vector<Route> routes;
		routes.reserve(fleet.trains.size());
		for (const Train *train : fleet.trains)
			routes.push_back(planOne(fleet, *train));
		return routes;
	}

	const char *name() const override { return "independent"; }

  private:
	static Route planOne(const FleetSnapshot &fleet, const Train &train)
	{
		Route path;
		try
		{
			if (fleet.mode == PathWeightMode::Adaptive)
				path = fleet.pathfinder.findPath(
					train.getDepartureStation(), train.getArrivalStation(),
					fleet.network, fleet.mode, SegmentOccupancy{}, {},
					fleet.eventRisk);
			else
				path = fleet.pathfinder.findPath(
					train.getDepartureStation(), train.getArrivalStation(),
					fleet.network, fleet.mode);
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
};

#endif
