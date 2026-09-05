/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simulation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 18:47:00 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <variant>
#include <vector>

#include "Event.hpp"
#include "FileOutputObserver.hpp"
#include "IPathfinding.hpp"
#include "ISimulationObserver.hpp"
#include "OutputManager.hpp"
#include "RailNetwork.hpp"
#include "Train.hpp"

/**
 * Live-mutation commands, safe to enqueue from another thread while
 * Simulation::run() is executing. Applied at the start of the next tick.
 */
struct AddNodeCommand
{
	std::string name;
};

struct AddRailCommand
{
	std::string from;
	std::string to;
	double distanceKm;
	double speedLimitKmh;
};

struct AddEventCommand
{
	std::string name;
	double probability;
	double durationSeconds;
	std::string node1;
	std::string node2;  // empty → station event
};

struct AddTrainCommand
{
	std::string name;
	double weightTons;
	double friction;
	double maxAccelKn;
	double maxBrakeKn;
	std::string from;
	std::string to;
	double departureTime;  // seconds from midnight
	double stopDuration;   // seconds
};

using LiveMutation = std::variant<AddNodeCommand, AddRailCommand,
								  AddEventCommand, AddTrainCommand>;

/**
 * Per-train runtime state used during the discrete simulation.
 */
struct TrainState
{
	Train *train;
	size_t segmentIndex;     // current segment (0 = first edge)
	double posOnSegment_m;   // metres from start of current segment
	double speed_ms;         // current speed in m/s
	double timeSinceDepart;  // seconds since this train departed
	double stopTimer;        // seconds remaining at a station stop
	bool departed;
	bool arrived;
	size_t segsSinceReroute; // segments elapsed since last congestion reroute
};

/**
 * Collected result for one train after a simulation run.
 */
struct TrainResult
{
	std::string name;
	int id;
	double estimatedTime;  // seconds
	double actualTime;     // seconds
	double totalDelay;     // seconds
};

/**
 * Callback signature for live animation displays.
 * Receives the current wall-clock simulation time and the full state vector.
 */
using AnimTickCallback = std::function<void(double simTime,
											const std::vector<TrainState> &)>;

class Simulation
{
  private:
	RailNetwork _network;
	std::vector<std::unique_ptr<Train>> _trains;
	std::vector<Event> _events;
	std::unique_ptr<IPathfinding> _pathfinder;
	PathWeightMode _weightMode;
	OutputManager _output;
	std::mt19937 _rng;
	std::vector<std::unique_ptr<ISimulationObserver>> _observers;
	std::vector<TrainResult> _results;
	AnimTickCallback _animCallback;
	bool _quiet;

	static constexpr double DT = 1.0;             // 1-second timestep
	static constexpr double OUTPUT_INTERVAL = 60.0; // output every minute
	static constexpr size_t REROUTE_COOLDOWN = 3;  // min segments between reroutes
	/* Floor applied to a train's speed when estimating how long it will
	   take to clear a segment, so a just-departed/near-stopped train
	   doesn't produce an inflated (near-infinite) clear-time estimate. */
	static constexpr double MIN_SPEED_FOR_ETA_MS = 1.0;

  public:
	Simulation(RailNetwork network,
			   std::vector<std::unique_ptr<Train>> trains,
			   std::vector<Event> events,
			   std::unique_ptr<IPathfinding> pathfinder,
			   PathWeightMode weightMode = PathWeightMode::Distance);
	Simulation(const Simulation &) = delete;
	Simulation &operator=(const Simulation &) = delete;
	~Simulation();

	void run();

	const RailNetwork &getNetwork() const;
	const std::vector<std::unique_ptr<Train>> &getTrains() const;
	const std::vector<TrainResult> &getResults() const;

	/** Set a per-tick callback for live animation (optional). */
	void setAnimCallback(AnimTickCallback cb);
	/** Suppress stdout output (useful for multi-run). */
	void setQuiet(bool q);

	/**
	 * Thread-safe: queue a mutation to be validated and applied at the
	 * start of the next tick. Safe to call from another thread while
	 * run() is executing (e.g. the GUI thread while the worker thread
	 * runs the physics loop).
	 */
	void enqueueMutation(LiveMutation mutation);

	/** Invoked (from the simulation thread) after a mutation is applied.
		Receives the mutation itself so callers can extract structured
		fields (e.g. to draw the new node/rail), plus a human-readable
		description for logging. */
	using MutationAppliedCallback =
		std::function<void(const LiveMutation &mutation,
						   const std::string &description)>;
	/** Invoked (from the simulation thread) when a mutation fails validation. */
	using MutationRejectedCallback = std::function<void(const std::string &reason)>;
	void setMutationCallbacks(MutationAppliedCallback onApplied,
							  MutationRejectedCallback onRejected);

  private:
	std::mutex _mutationMutex;
	std::vector<LiveMutation> _pendingMutations;
	MutationAppliedCallback _onMutationApplied;
	MutationRejectedCallback _onMutationRejected;

	std::vector<LiveMutation> drainMutations();
	/* States/observers live in run()'s stack frame; passed by reference
	   so AddTrainCommand can append a new TrainState/observer mid-run. */
	void applyMutation(const LiveMutation &mutation,
					   std::vector<TrainState> &states, double simTime);
	void applyAddNode(const AddNodeCommand &c);
	void applyAddRail(const AddRailCommand &c);
	void applyAddEvent(const AddEventCommand &c);
	void applyAddTrain(const AddTrainCommand &c,
					   std::vector<TrainState> &states, double simTime);

	void computePaths();
	void staticPreAssign();
	double estimateTravelTime(const Train &train) const;
	void getSegmentInfo(const std::string &from, const std::string &to,
						double &length_m, double &speedLimit_ms) const;
	double totalRemainingDistance(const TrainState &s) const;
	void updatePhysics(TrainState &s);
	void handleSegmentTransition(TrainState &s, size_t trainIdx,
								 const std::vector<TrainState> &states,
								 SegmentOccupancy &tickOccupancy,
								 SegmentClearTimes &tickClearTimes,
								 const SegmentEventRisk &eventRisk);
	void applyBlocking(std::vector<TrainState> &states);
	SegmentOccupancy buildOccupancy(
		const std::vector<TrainState> &states,
		size_t excludeIdx = SIZE_MAX) const;
	/** Estimated seconds for each occupied segment's current occupant(s)
		to clear it (max across occupants sharing that segment). */
	SegmentClearTimes buildClearTimes(
		const std::vector<TrainState> &states,
		size_t excludeIdx = SIZE_MAX) const;
	/** Expected delay (seconds) from random events bound to each segment
		or its destination node — static per network, independent of
		current traffic. Computed fresh each tick so live-added events
		(via enqueueMutation) are picked up immediately. */
	SegmentEventRisk buildEventRisk() const;
	bool hasCongestedSegmentAhead(const TrainState &s,
								 const SegmentOccupancy &occupancy) const;
	bool wouldBeBlocked(const TrainState &s,
						const std::vector<TrainState> &states) const;
	double computePathCost(
		const std::vector<std::shared_ptr<Node>> &path,
		size_t startIdx,
		const SegmentOccupancy &occupancy,
		const SegmentClearTimes &clearTimes,
		const SegmentEventRisk &eventRisk) const;
	void rerouteFromNode(TrainState &s,
						 SegmentOccupancy &occupancy,
						 SegmentClearTimes &clearTimes,
						 const SegmentEventRisk &eventRisk);
	std::vector<const Event *> getEventsAtNode(
		const std::string &nodeName) const;
	std::vector<const Event *> getEventsOnSegment(
		const std::string &from, const std::string &to) const;
};

#endif
