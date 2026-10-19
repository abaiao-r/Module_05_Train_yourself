/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simulation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/06 12:18:34 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
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
	/* timeSinceDepart at the moment this train entered its CURRENT
	   segment - used to determine right-of-way in applyBlocking() by
	   entry order, since raw position is not a stable ordering (it can
	   only ever reflect "who is ahead right now", which is exactly
	   what an illegal overtake would corrupt). */
	double segmentEntryTime;
	/* Which train is currently capping this train's speed, if any -
	   a per-tick fact, recomputed and cleared every tick by
	   applyBlocking(). Null when not blocked. */
	Train *blockedByTrain = nullptr;
	/* Total successful reroutes so far, and a short human-readable
	   reason for the most recent one (empty if never rerouted). */
	int rerouteCount = 0;
	std::string lastRerouteReason;

	/* Free-flow predicted arrival (absolute sim seconds) at each node
	   after the current segment, i.e. path[segmentIndex+1..]. "Ideal,
	   ignoring traffic" - GUI display only, never a decision input.
	   Only maintained while an animation callback is set. */
	std::vector<double> etaProfile;
	bool profileDirty = true;
	double lastProfileTime = -1e18;

	/* Reroute-evaluation bookkeeping (Adaptive mode). Budget: at most
	   one entry, one event-triggered and one pre-pivot evaluation per
	   segment. */
	unsigned routeVersion = 0;
	bool evalEntryDone = false;
	bool evalEventDone = false;
	bool evalPrePivotDone = false;
	const Train *lastLeader = nullptr;
	unsigned lastLeaderVersion = 0;
	unsigned lastGen = 0;
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
/** Invoked whenever a random event actually triggers on a train, for
	displays that want to show it live (e.g. a flash on the graph),
	separate from onTrainEvent() which is the file-output observer path. */
using EventTriggerCallback = std::function<void(
	const std::string &trainName, const std::string &eventName,
	const std::string &location, double delaySec)>;
/** Invoked whenever a train's path is successfully changed by the
	adaptive reroute logic. */
using RerouteCallback = std::function<void(
	const std::string &trainName, const std::string &fromNode,
	const std::string &newRouteDescription)>;

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
	EventTriggerCallback _eventTriggerCallback;
	RerouteCallback _rerouteCallback;
	bool _quiet;

	/* Monotonic counters bumped by live mutations; part of the
	   candidate-path cache key. */
	unsigned _topologyGen = 0;
	unsigned _eventGen = 0;
	unsigned _cacheGen = ~0u;
	mutable SegmentEventRisk _riskCache;
	mutable unsigned _riskCacheGen = ~0u;
	std::unordered_map<std::string, std::vector<std::string>> _candidateCache;

	static constexpr double DT = 1.0;             // 1-second timestep
	static constexpr double OUTPUT_INTERVAL = 60.0; // output every minute
	static constexpr size_t REROUTE_COOLDOWN = 3;  // min segments between reroutes
	/* Floor applied to a train's speed when estimating how long it will
	   take to clear a segment, so a just-departed/near-stopped train
	   doesn't produce an inflated (near-infinite) clear-time estimate. */
	static constexpr double MIN_SPEED_FOR_ETA_MS = 1.0;
	/* Minimum safe following distance on a shared segment - a follower
	   is clamped to stay at least this far behind the leader, not just
	   short of literally the same point (real trains need a safety
	   gap, not a zero-distance clamp). */
	static constexpr double MIN_SAFE_FOLLOWING_DISTANCE_M = 200.0;
	/* A detour must beat staying put by at least this many seconds. */
	static constexpr double MIN_GAIN_SEC = 30.0;
	/* Pre-pivot re-evaluation horizon (seconds before the next node). */
	static constexpr double PRE_PIVOT_HORIZON_SEC = 60.0;
	/* Minimum sim seconds between deviation-triggered profile rebuilds. */
	static constexpr double PROFILE_REBUILD_MIN_SEC = 10.0;
	static constexpr double REPLAY_MAX_SEC = 36000.0;

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
	/** Static event list (name, probability, duration, node(s)) - for
		displays that want to show event risk before it materializes. */
	const std::vector<Event> &getEvents() const;

	/** Set a per-tick callback for live animation (optional). */
	void setAnimCallback(AnimTickCallback cb);
	/** Suppress stdout output (useful for multi-run). */
	void setQuiet(bool q);

	/** Invoked (from the simulation thread) whenever a random event
		actually triggers on a train, for displays that want to show it
		live (e.g. a flash on the graph), separate from onTrainEvent()
		which is the file-output observer path. */
	void setEventTriggerCallback(EventTriggerCallback cb);
	/** Invoked (from the simulation thread) whenever a train's path is
		successfully changed by the adaptive reroute logic. */
	void setRerouteCallback(RerouteCallback cb);

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
	/** One 1-second physics step along `path`; shared by the real
		simulation and the virtual ETA replay so they cannot drift. */
	void physicsStep(const std::vector<std::shared_ptr<Node>> &path,
					 size_t segIdx, double &pos, double &speed,
					 double accel, double decel) const;
	/** Exact kinematic ETA to cover a remaining distance from a given
		current speed under constant acceleration, capped at the
		segment's speed limit - deterministic, not an approximation. */
	double kinematicEta(double currentSpeed_ms, double accel_ms2,
						double remainingDist_m, double speedLimit_ms) const;

	/** Virtual (copy) train state for non-mutating ETA replay. */
	struct VTrain
	{
		const std::vector<std::shared_ptr<Node>> *path;
		size_t seg;
		double pos;
		double speed;
		double stopTimer;
		double accel;
		double decel;
		double stopDur;
		double entryT; // relative to the replay start (may be negative)
		bool done;
	};
	/** Replays the real physics/transition/blocking rules on copies and
		returns seconds until `me` arrives. `leader` (optional) is
		advanced with free physics and caps `me` on shared directed
		segments it entered first. If `nodeTimes` is given it receives
		the arrival time at each node after the current segment. */
	double replayEta(VTrain me, const VTrain *leader,
					 std::vector<double> *nodeTimes = nullptr) const;
	VTrain makeVTrain(const TrainState &s, double simTime) const;

	/** Occupants of each directed segment ("from->to"), ordered by right
		of way (earliest segmentEntryTime, then index). Dwelling,
		arrived and not-yet-departed trains are excluded. */
	using SegmentState = std::unordered_map<std::string, std::vector<size_t>>;
	SegmentState buildSegmentState(const std::vector<TrainState> &states) const;
	/** Cheap per-tick predicate + capped expensive evaluation. */
	void considerReroute(std::vector<TrainState> &states, size_t idx,
						 const std::vector<size_t> &group, size_t posInGroup,
						 double simTime);
	bool evaluateDetour(TrainState &s, const TrainState &leader,
						size_t sharedLen, double simTime);
	void rebuildProfile(TrainState &s, double simTime);

	void rollSegmentEvents(TrainState &s, size_t trainIdx);
	void handleSegmentTransition(TrainState &s, size_t trainIdx,
								 double simTime);
	void applyBlocking(std::vector<TrainState> &states,
					   const SegmentState &segState, double simTime);
	/** Expected delay (seconds) from random events bound to each segment
		or its destination node - static per network, independent of
		current traffic. */
	SegmentEventRisk buildEventRisk() const;
	const SegmentEventRisk &cachedEventRisk() const;
	std::vector<const Event *> getEventsAtNode(
		const std::string &nodeName) const;
	std::vector<const Event *> getEventsOnSegment(
		const std::string &from, const std::string &to) const;
};

#endif
