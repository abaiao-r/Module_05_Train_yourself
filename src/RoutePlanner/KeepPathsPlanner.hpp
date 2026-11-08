#ifndef KEEPPATHSPLANNER_HPP
#define KEEPPATHSPLANNER_HPP

#include "IRoutePlanner.hpp"

/** Returns every train's current path unchanged (used by dry runs that
	must replay routes some other planner already chose). */
class KeepPathsPlanner : public IRoutePlanner
{
  public:
	std::vector<Route> planAll(const FleetSnapshot &fleet) override
	{
		std::vector<Route> out;
		for (const Train *t : fleet.trains)
			out.push_back(t->getPath());
		return out;
	}

	const char *name() const override { return "keep"; }
};

#endif
