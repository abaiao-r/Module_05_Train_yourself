/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simulation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 15:23:38 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATION_HPP
#define SIMULATION_HPP

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
};

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

	static constexpr double DT = 1.0;             // 1-second timestep
	static constexpr double OUTPUT_INTERVAL = 60.0; // output every minute

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

  private:
	void computePaths();
	double estimateTravelTime(const Train &train) const;
	void getSegmentInfo(const std::string &from, const std::string &to,
						double &length_m, double &speedLimit_ms) const;
	double totalRemainingDistance(const TrainState &s) const;
	void updatePhysics(TrainState &s);
	void handleSegmentTransition(TrainState &s, size_t trainIdx);
	void applyBlocking(std::vector<TrainState> &states);
	std::vector<const Event *> getEventsAtNode(
		const std::string &nodeName) const;
};

#endif
