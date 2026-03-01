/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simulation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/03/01 19:20:41 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <functional>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "Event.hpp"
#include "FileOutputObserver.hpp"
#include "IPathfinding.hpp"
#include "ISimulationObserver.hpp"
#include "OutputManager.hpp"
#include "RailNetwork.hpp"
#include "Train.hpp"

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

  private:
	void computePaths();
	double estimateTravelTime(const Train &train) const;
	void getSegmentInfo(const std::string &from, const std::string &to,
						double &length_m, double &speedLimit_ms) const;
	double totalRemainingDistance(const TrainState &s) const;
	void updatePhysics(TrainState &s);
	void handleSegmentTransition(TrainState &s, size_t trainIdx,
								 const std::vector<TrainState> &states,
								 const SegmentOccupancy &tickOccupancy);
	void applyBlocking(std::vector<TrainState> &states);
	SegmentOccupancy buildOccupancy(
		const std::vector<TrainState> &states,
		size_t excludeIdx = SIZE_MAX) const;
	bool hasCongestedSegmentAhead(const TrainState &s,
								 const SegmentOccupancy &occupancy) const;
	void rerouteFromNode(TrainState &s,
						 const SegmentOccupancy &occupancy);
	std::vector<const Event *> getEventsAtNode(
		const std::string &nodeName) const;
	std::vector<const Event *> getEventsOnSegment(
		const std::string &from, const std::string &to) const;
};

#endif
