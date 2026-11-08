#ifndef IROUTEPLANNER_HPP
#define IROUTEPLANNER_HPP

#include <memory>
#include <vector>

#include "IPathfinding.hpp"
#include "Node.hpp"
#include "RailNetwork.hpp"
#include "Train.hpp"

using Route = std::vector<std::shared_ptr<Node>>;

/**
 * Read-only view of the fleet handed to a planner: everything it may use
 * to choose routes, nothing it may change.
 */
struct FleetSnapshot
{
	const RailNetwork &network;
	std::vector<const Train *> trains;
	PathWeightMode mode;
	SegmentEventRisk eventRisk; // empty unless mode is Adaptive
	IPathfinding &pathfinder;   // per-train worker
	bool lanesEnabled = false;  // --lines: head-on exclusion per lane
};

/**
 * Fleet-level Strategy: decides every train's route together.
 * IPathfinding answers "best path for ONE train"; an IRoutePlanner
 * answers "best set of paths for ALL trains".
 */
class IRoutePlanner
{
  public:
	virtual ~IRoutePlanner() = default;

	/** One route per snapshot train, same order. Throws
		std::runtime_error("No path found for ...") if a train has none. */
	virtual std::vector<Route> planAll(const FleetSnapshot &fleet) = 0;

	virtual const char *name() const = 0;
};

#endif
