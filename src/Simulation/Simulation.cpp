/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simulation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 19:17:55 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Simulation.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <type_traits>
#include <utility>

#include "DijkstraPathfinding.hpp"
#include "Node.hpp"
#include "TrainFactory.hpp"

/* ---- Constructor / Destructor ---- */
Simulation::Simulation(RailNetwork network,
					   std::vector<std::unique_ptr<Train>> trains,
					   std::vector<Event> events,
					   std::unique_ptr<IPathfinding> pathfinder,
					   PathWeightMode weightMode)
	: _network(std::move(network)), _trains(std::move(trains)),
	  _events(std::move(events)), _pathfinder(std::move(pathfinder)),
	  _weightMode(weightMode), _rng(std::random_device{}()),
	  _quiet(false)
{
	if (!_pathfinder)
		throw std::invalid_argument("Pathfinder cannot be null");
}

Simulation::~Simulation() {}

/* ---- Const accessors (for auxiliary tools) ---- */
const RailNetwork &Simulation::getNetwork() const { return _network; }

const std::vector<std::unique_ptr<Train>> &Simulation::getTrains() const
{
	return _trains;
}

const std::vector<TrainResult> &Simulation::getResults() const
{
	return _results;
}

void Simulation::setAnimCallback(AnimTickCallback cb)
{
	_animCallback = std::move(cb);
}

void Simulation::setQuiet(bool q) { _quiet = q; }

/* ---- Live mutation queue ---- */
void Simulation::enqueueMutation(LiveMutation mutation)
{
	std::lock_guard<std::mutex> lock(_mutationMutex);
	_pendingMutations.push_back(std::move(mutation));
}

void Simulation::setMutationCallbacks(MutationAppliedCallback onApplied,
									  MutationRejectedCallback onRejected)
{
	_onMutationApplied = std::move(onApplied);
	_onMutationRejected = std::move(onRejected);
}

std::vector<LiveMutation> Simulation::drainMutations()
{
	std::lock_guard<std::mutex> lock(_mutationMutex);
	std::vector<LiveMutation> batch = std::move(_pendingMutations);
	_pendingMutations.clear();
	return batch;
}

void Simulation::applyMutation(const LiveMutation &mutation,
							   std::vector<TrainState> &states,
							   double simTime)
{
	try
	{
		std::visit(
			[&](const auto &cmd) {
				using T = std::decay_t<decltype(cmd)>;
				if constexpr (std::is_same_v<T, AddNodeCommand>)
				{
					applyAddNode(cmd);
					if (_onMutationApplied)
						_onMutationApplied(mutation, "Added node: " + cmd.name);
				}
				else if constexpr (std::is_same_v<T, AddRailCommand>)
				{
					applyAddRail(cmd);
					if (_onMutationApplied)
						_onMutationApplied(mutation, "Added rail: " + cmd.from
											+ " <-> " + cmd.to);
				}
				else if constexpr (std::is_same_v<T, AddEventCommand>)
				{
					applyAddEvent(cmd);
					if (_onMutationApplied)
						_onMutationApplied(mutation, "Added event: " + cmd.name);
				}
				else if constexpr (std::is_same_v<T, AddTrainCommand>)
				{
					applyAddTrain(cmd, states, simTime);
					if (_onMutationApplied)
						_onMutationApplied(mutation, "Added train: " + cmd.name);
				}
			},
			mutation);
	}
	catch (const std::exception &e)
	{
		if (_onMutationRejected)
			_onMutationRejected(e.what());
	}
}

void Simulation::applyAddNode(const AddNodeCommand &c)
{
	_network.addNode(c.name);
}

void Simulation::applyAddRail(const AddRailCommand &c)
{
	_network.addConnection(c.from, c.to, c.distanceKm, c.speedLimitKmh);
}

void Simulation::applyAddEvent(const AddEventCommand &c)
{
	_network.findNode(c.node1);
	if (!c.node2.empty())
		_network.findNode(c.node2);
	_events.emplace_back(c.name, c.probability, c.durationSeconds,
						 c.node1, c.node2);
}

void Simulation::applyAddTrain(const AddTrainCommand &c,
							   std::vector<TrainState> &states,
							   double simTime)
{
	if (c.departureTime < simTime)
		throw std::invalid_argument(
			"Departure time has already passed (train '" + c.name + "')");

	auto train = TrainFactory::createTrain(
		c.name, c.weightTons, c.friction, c.maxAccelKn, c.maxBrakeKn,
		c.from, c.to, c.departureTime, c.stopDuration);

	auto path = _pathfinder->findPath(c.from, c.to, _network, _weightMode);
	if (path.empty())
		throw std::runtime_error("No route from " + c.from + " to " + c.to
								 + " for train '" + c.name + "'");
	train->setPath(path);

	TrainState s{};
	s.train = train.get();
	s.segmentIndex = 0;
	s.posOnSegment_m = 0.0;
	s.speed_ms = 0.0;
	s.timeSinceDepart = 0.0;
	s.stopTimer = 0.0;
	s.departed = false;
	s.arrived = false;
	states.push_back(s);

	auto obs = std::make_unique<FileOutputObserver>(train->getName(),
													train->getDepartureTime());
	obs->onTrainStart(train->getName(), train->getId(),
					  estimateTravelTime(*train));
	_observers.push_back(std::move(obs));

	_trains.push_back(std::move(train));
}

/* ---- Public ---- */
void Simulation::run()
{
	if (!_quiet)
		_output.printNetwork(_network);
	computePaths();

	if (!_quiet)
		std::cout << "=== Simulation ===" << std::endl;

	/* Sort trains by departure time */
	std::sort(_trains.begin(), _trains.end(),
			  [](const auto &a, const auto &b) {
				  return a->getDepartureTime() < b->getDepartureTime();
			  });

	/* Create per-train states and file-output observers */
	std::vector<TrainState> states;
	states.reserve(_trains.size());
	_observers.clear();

	for (auto &train : _trains)
	{
		TrainState s{};
		s.train = train.get();
		s.segmentIndex = 0;
		s.posOnSegment_m = 0.0;
		s.speed_ms = 0.0;
		s.timeSinceDepart = 0.0;
		s.stopTimer = 0.0;
		s.departed = false;
		s.arrived = false;
		s.segsSinceReroute = REROUTE_COOLDOWN; // allow immediate first reroute
		states.push_back(s);

		auto obs = std::make_unique<FileOutputObserver>(
			train->getName(), train->getDepartureTime());
		_observers.push_back(std::move(obs));
	}

	/* Write estimated travel times to result files */
	for (size_t i = 0; i < states.size(); i++)
	{
		double est = estimateTravelTime(*states[i].train);
		_observers[i]->onTrainStart(states[i].train->getName(),
									states[i].train->getId(), est);
	}

	/* Find earliest departure to start the clock */
	double simTime = std::numeric_limits<double>::max();
	for (auto &t : _trains)
	{
		if (t->getDepartureTime() < simTime)
			simTime = t->getDepartureTime();
	}
	simTime -= DT;

	/* Main concurrent simulation loop */
	auto anyActive = [&]() {
		for (auto &s : states)
		{
			if (!s.arrived)
				return true;
		}
		return false;
	};

	while (anyActive())
	{
		simTime += DT;

		/* Apply any live mutations queued from another thread (e.g. the
		   GUI) before running physics for this tick. */
		for (const auto &mutation : drainMutations())
			applyMutation(mutation, states, simTime);

		/* Build occupancy snapshot once per tick (adaptive mode only).
		   Both maps are mutated in place as trains reroute within this
		   tick, so later trains in the same tick see earlier trains'
		   just-made choices instead of a stale tick-start snapshot
		   (avoids several trains piling onto the same "empty"
		   alternate corridor at once). Event risk is static, but still
		   rebuilt each tick so live-added events are picked up. */
		SegmentOccupancy tickOccupancy;
		SegmentClearTimes tickClearTimes;
		SegmentEventRisk tickEventRisk;
		if (_weightMode == PathWeightMode::Adaptive)
		{
			tickOccupancy = buildOccupancy(states);
			tickClearTimes = buildClearTimes(states);
			tickEventRisk = buildEventRisk();
		}

		for (size_t i = 0; i < states.size(); i++)
		{
			auto &s = states[i];
			if (s.arrived)
				continue;

			/* Check departure */
			if (!s.departed)
			{
				if (simTime >= s.train->getDepartureTime())
				{
					s.departed = true;
					s.train->setStatus(TrainStatus::Running);
					if (!_quiet)
						_output.printDeparture(*s.train);
				}
				continue;
			}

			/* Station stop countdown */
			bool isStopped = false;
			if (s.stopTimer > 0.0)
			{
				s.stopTimer -= DT;
				if (s.stopTimer <= 0.0)
					s.stopTimer = 0.0;
				s.timeSinceDepart += DT;
				s.train->setCurrentTime(s.train->getCurrentTime() + DT);
				isStopped = true;
			}

			if (!isStopped)
			{
				/* Skip if path too short */
				if (s.train->getPath().size() < 2)
				{
					s.arrived = true;
					s.train->setStatus(TrainStatus::Arrived);
					continue;
				}

				/* Physics step */
				updatePhysics(s);
				s.timeSinceDepart += DT;
				s.train->setCurrentTime(s.train->getCurrentTime()
										+ DT);

				/* Segment transition */
				auto &path = s.train->getPath();
				double segLen_m, segSpd_ms;
				getSegmentInfo(path[s.segmentIndex]->getName(),
							   path[s.segmentIndex + 1]->getName(),
							   segLen_m, segSpd_ms);

				if (s.posOnSegment_m >= segLen_m)
					handleSegmentTransition(s, i, states, tickOccupancy,
											tickClearTimes, tickEventRisk);
			}

			/* Output at intervals */
			double modt = std::fmod(s.timeSinceDepart, OUTPUT_INTERVAL);
			if (modt < DT || s.arrived)
			{
				double distRem = totalRemainingDistance(s) / 1000.0;
				auto &p = s.train->getPath();
				if (p.size() >= 2 && s.segmentIndex + 1 < p.size())
				{
					const std::string &from =
						p[s.segmentIndex]->getName();
					const std::string &to =
						p[s.segmentIndex + 1]->getName();
					double sLen_m, sSp_ms;
					getSegmentInfo(from, to, sLen_m, sSp_ms);
					int segKm = static_cast<int>(std::ceil(
						sLen_m / 1000.0));
					if (segKm < 1)
						segKm = 1;
					int posKm = static_cast<int>(
						s.posOnSegment_m / 1000.0);
					if (posKm >= segKm)
						posKm = segKm - 1;

					/* Find blocking trains on same segment */
					std::vector<int> blocking;
					for (size_t j = 0; j < states.size(); j++)
					{
						if (j == i || states[j].arrived
							|| !states[j].departed)
							continue;
						auto &op = states[j].train->getPath();
						if (op.size() >= 2
							&& states[j].segmentIndex + 1 < op.size()
							&& op[states[j].segmentIndex]->getName()
								   == from
							&& op[states[j].segmentIndex + 1]->getName()
								   == to)
						{
							int oPos = static_cast<int>(
								states[j].posOnSegment_m / 1000.0);
							if (oPos >= segKm)
								oPos = segKm - 1;
							blocking.push_back(oPos);
						}
					}

					std::string action;
					if (isStopped)
						action = " Stopped";
					else if (s.arrived)
						action = " Stopped";
					else
					{
						double brakeDist = (s.speed_ms * s.speed_ms)
										   / (2.0
											  * s.train->getDecelRate());
						double distToEnd = sLen_m - s.posOnSegment_m;
						if (distToEnd <= brakeDist + 5.0
							&& s.speed_ms > 0.1)
							action = " Braking";
						else if (s.speed_ms < sSp_ms - 0.5)
							action = "Speed up";
						else
							action = "Maintain";
					}

					_observers[i]->onTrainStep(
						s.timeSinceDepart, from, to, distRem, action,
						posKm, segKm, blocking);
				}
			}
		}

		/* Overtaking / blocking check */
		applyBlocking(states);

		/* Live animation callback */
		if (_animCallback)
			_animCallback(simTime, states);
	}

	/* Drain any mutations queued right as the last train arrived, so a
	   structural edit (e.g. AddNode) enqueued at the very last tick isn't
	   silently dropped. New trains added here won't get physics ticks. */
	for (const auto &mutation : drainMutations())
		applyMutation(mutation, states, simTime);

	/* Finalize observer files */
	for (size_t i = 0; i < states.size(); i++)
	{
		_observers[i]->onTrainFinish(states[i].train->getName(),
									 states[i].timeSinceDepart);
	}

	if (!_quiet)
		_output.printResult(_trains);

	/* Collect per-train results */
	_results.clear();
	for (size_t i = 0; i < states.size(); i++)
	{
		TrainResult r;
		r.name = states[i].train->getName();
		r.id = states[i].train->getId();
		r.estimatedTime = estimateTravelTime(*states[i].train);
		r.actualTime = states[i].timeSinceDepart;
		r.totalDelay = states[i].train->getTotalDelay();
		_results.push_back(r);
	}
}

/* ---- Path computation ---- */
void Simulation::computePaths()
{
	if (!_quiet)
		std::cout << "=== Paths ===" << std::endl;

	/* Event risk is static (independent of current traffic), so Adaptive
	   mode avoids event-prone routes from the very first path assignment,
	   not just later reroutes. */
	SegmentEventRisk eventRisk;
	if (_weightMode == PathWeightMode::Adaptive)
		eventRisk = buildEventRisk();

	for (auto &train : _trains)
	{
		std::vector<std::shared_ptr<Node>> path;
		try
		{
			if (_weightMode == PathWeightMode::Adaptive)
				path = _pathfinder->findPath(train->getDepartureStation(),
											 train->getArrivalStation(),
											 _network, _weightMode,
											 SegmentOccupancy{}, {},
											 eventRisk);
			else
				path = _pathfinder->findPath(train->getDepartureStation(),
											 train->getArrivalStation(),
											 _network, _weightMode);
		}
		catch (const std::exception &e)
		{
			std::cerr << "No path for " << train->getName() << ": "
					  << e.what() << std::endl;
			continue;
		}
		if (path.empty())
		{
			std::cerr << "No path found for " << train->getName() << " ("
					  << train->getDepartureStation() << " -> "
					  << train->getArrivalStation() << ")" << std::endl;
			continue;
		}
		train->setPath(path);
		if (!_quiet)
			_output.printTrainPath(*train);
	}
	if (!_quiet)
		std::cout << std::endl;
}

/* ---- Estimated travel time (analytical) ---- */
double Simulation::estimateTravelTime(const Train &train) const
{
	const auto &path = train.getPath();
	if (path.size() < 2)
		return 0.0;

	double totalTime = 0.0;
	double accel = train.getAccelRate();
	double decel = train.getDecelRate();

	for (size_t i = 0; i < path.size() - 1; i++)
	{
		double segLen_m, speedLim_ms;
		getSegmentInfo(path[i]->getName(), path[i + 1]->getName(),
					   segLen_m, speedLim_ms);

		/* Solo to accelerate from 0 to speed limit */
		double tAccel = speedLim_ms / accel;
		double dAccel = 0.5 * accel * tAccel * tAccel;

		/* Solo to brake from speed limit to 0 */
		double tBrake = speedLim_ms / decel;
		double dBrake = 0.5 * decel * tBrake * tBrake;

		if (dAccel + dBrake > segLen_m)
		{
			/* Segment too short to reach full speed */
			double vMax = std::sqrt(
				2.0 * segLen_m * accel * decel / (accel + decel));
			totalTime += vMax / accel + vMax / decel;
		}
		else
		{
			double dMaintain = segLen_m - dAccel - dBrake;
			double tMaintain = dMaintain / speedLim_ms;
			totalTime += tAccel + tMaintain + tBrake;
		}

		/* Add stop duration at intermediate stations */
		if (i < path.size() - 2)
			totalTime += train.getStopDuration();
	}
	return totalTime;
}

/* ---- Segment info lookup ---- */
void Simulation::getSegmentInfo(const std::string &from,
								const std::string &to, double &length_m,
								double &speedLimit_ms) const
{
	const auto &edges = _network.getNeighbours(from);
	for (const auto &edge : edges)
	{
		if (edge.destination->getName() == to)
		{
			length_m = edge.distance * 1000.0;
			speedLimit_ms = edge.speedLimit / 3.6;
			return;
		}
	}
	throw std::runtime_error("No edge from " + from + " to " + to);
}

/* ---- Total remaining distance from current position ---- */
double Simulation::totalRemainingDistance(const TrainState &s) const
{
	const auto &path = s.train->getPath();
	if (path.size() < 2 || s.segmentIndex + 1 >= path.size())
		return 0.0;

	/* Remaining on current segment */
	double segLen_m, spd_ms;
	getSegmentInfo(path[s.segmentIndex]->getName(),
				   path[s.segmentIndex + 1]->getName(), segLen_m, spd_ms);
	double remaining = segLen_m - s.posOnSegment_m;

	/* Full lengths of subsequent segments */
	for (size_t i = s.segmentIndex + 1; i + 1 < path.size(); i++)
	{
		double sLen, sSp;
		getSegmentInfo(path[i]->getName(), path[i + 1]->getName(), sLen,
					   sSp);
		remaining += sLen;
	}
	return remaining;
}

/* ---- Physics step (1 second) ---- */
void Simulation::updatePhysics(TrainState &s)
{
	auto &path = s.train->getPath();
	if (path.size() < 2 || s.segmentIndex + 1 >= path.size())
		return;

	double segLen_m, speedLim_ms;
	getSegmentInfo(path[s.segmentIndex]->getName(),
				   path[s.segmentIndex + 1]->getName(), segLen_m,
				   speedLim_ms);

	double accel = s.train->getAccelRate();
	double decel = s.train->getDecelRate();
	double distToEnd = segLen_m - s.posOnSegment_m;

	/* Determine target end speed: 0 at last segment, else next limit */
	double targetEnd = 0.0;
	bool isLast = (s.segmentIndex == path.size() - 2);
	if (!isLast)
	{
		double nLen, nSpd;
		getSegmentInfo(path[s.segmentIndex + 1]->getName(),
					   path[s.segmentIndex + 2]->getName(), nLen, nSpd);
		targetEnd = nSpd;
		if (targetEnd > speedLim_ms)
			targetEnd = speedLim_ms;
	}

	/* Braking distance needed to reach targetEnd */
	double dv = s.speed_ms - targetEnd;
	double brakeDist = 0.0;
	if (dv > 0.0)
		brakeDist = (s.speed_ms * s.speed_ms - targetEnd * targetEnd)
					/ (2.0 * decel);

	/* Decision */
	double newSpeed = s.speed_ms;
	if (distToEnd <= brakeDist + 2.0 && s.speed_ms > targetEnd + 0.1)
	{
		/* Brake */
		newSpeed = s.speed_ms - decel * DT;
		if (newSpeed < targetEnd)
			newSpeed = targetEnd;
		if (newSpeed < 0.0)
			newSpeed = 0.0;
	}
	else if (s.speed_ms < speedLim_ms - 0.1)
	{
		/* Accelerate */
		newSpeed = s.speed_ms + accel * DT;
		if (newSpeed > speedLim_ms)
			newSpeed = speedLim_ms;
	}
	else
	{
		/* Maintain */
		newSpeed = speedLim_ms;
	}

	/* Update position */
	double avgSpeed = (s.speed_ms + newSpeed) / 2.0;
	double dist = avgSpeed * DT;
	s.posOnSegment_m += dist;
	s.speed_ms = newSpeed;

	/* Clamp */
	if (s.posOnSegment_m > segLen_m)
		s.posOnSegment_m = segLen_m;
}

/* ---- Segment transition ---- */
void Simulation::handleSegmentTransition(TrainState &s, size_t trainIdx,
										 const std::vector<TrainState> &states,
										 SegmentOccupancy &tickOccupancy,
										 SegmentClearTimes &tickClearTimes,
										 const SegmentEventRisk &eventRisk)
{
	auto &path = s.train->getPath();
	if (path.size() < 2)
		return;
	bool isLast = (s.segmentIndex + 2 >= path.size());

	/* Notify arrival at next node */
	std::string arrNode = path[s.segmentIndex + 1]->getName();
	if (!_quiet)
		_output.printArrival(*s.train, arrNode, s.train->getCurrentTime());

	/* Trigger events at this node */
	auto nodeEvents = getEventsAtNode(arrNode);
	for (const auto *event : nodeEvents)
	{
		if (event->tryTrigger(_rng))
		{
			double delay = event->getDuration();
			s.train->applyDelay(delay);
			s.timeSinceDepart += delay;
			if (!_quiet)
				_output.printEvent(*event, *s.train);
			_observers[trainIdx]->onTrainEvent(
				event->getName(), event->getNodeName(), delay);
		}
	}

	if (isLast)
	{
		/* Arrived at destination */
		s.speed_ms = 0.0;
		s.arrived = true;
		s.train->setStatus(TrainStatus::Arrived);
	}
	else
	{
		/* Move to next segment */
		s.segmentIndex++;
		s.posOnSegment_m = 0.0;
		s.train->setPathIndex(s.segmentIndex);

		/* Trigger rail-segment events on the new segment */
		if (s.segmentIndex + 1 < path.size())
		{
			std::string segFrom = path[s.segmentIndex]->getName();
			std::string segTo = path[s.segmentIndex + 1]->getName();
			auto railEvents = getEventsOnSegment(segFrom, segTo);
			for (const auto *event : railEvents)
			{
				if (event->tryTrigger(_rng))
				{
					double delay = event->getDuration();
					s.train->applyDelay(delay);
					s.timeSinceDepart += delay;
					if (!_quiet)
						_output.printEvent(*event, *s.train);
					std::string loc = event->getNodeName() + "-"
									  + event->getNodeName2();
					_observers[trainIdx]->onTrainEvent(
						event->getName(), loc, delay);
				}
			}
		}

		/* Apply stop duration at intermediate stations */
		s.stopTimer = s.train->getStopDuration();

		/* Adaptive-aware re-routing from the new current node.
		   Note: tickOccupancy was built at tick start (before transitions),
		   so this train is counted on its OLD segment, not its new one.
		   No self-exclusion is needed. Both maps are updated in place by
		   rerouteFromNode() when it succeeds, so later trains processed
		   in this same tick see this train's new choice. */
		s.segsSinceReroute++;
		if (_weightMode == PathWeightMode::Adaptive
			&& s.segsSinceReroute >= REROUTE_COOLDOWN
			&& hasCongestedSegmentAhead(s, tickOccupancy)
			&& wouldBeBlocked(s, states))
		{
			rerouteFromNode(s, tickOccupancy, tickClearTimes, eventRisk);
		}
	}
}

/* ---- Build segment occupancy map ---- */
SegmentOccupancy Simulation::buildOccupancy(
	const std::vector<TrainState> &states, size_t excludeIdx) const
{
	SegmentOccupancy occ;
	for (size_t i = 0; i < states.size(); i++)
	{
		if (i == excludeIdx)
			continue;
		const auto &ts = states[i];
		if (ts.arrived || !ts.departed)
			continue;
		const auto &p = ts.train->getPath();
		if (p.size() < 2 || ts.segmentIndex + 1 >= p.size())
			continue;
		std::string key = p[ts.segmentIndex]->getName() + "->"
						  + p[ts.segmentIndex + 1]->getName();
		occ[key]++;
	}
	return occ;
}

/* ---- Build estimated clear-time map (time-aware congestion cost) ---- */
SegmentClearTimes Simulation::buildClearTimes(
	const std::vector<TrainState> &states, size_t excludeIdx) const
{
	SegmentClearTimes clearTimes;
	for (size_t i = 0; i < states.size(); i++)
	{
		if (i == excludeIdx)
			continue;
		const auto &ts = states[i];
		if (ts.arrived || !ts.departed)
			continue;
		const auto &p = ts.train->getPath();
		if (p.size() < 2 || ts.segmentIndex + 1 >= p.size())
			continue;

		double segLen_m, segSpd_ms;
		getSegmentInfo(p[ts.segmentIndex]->getName(),
					   p[ts.segmentIndex + 1]->getName(),
					   segLen_m, segSpd_ms);
		double remaining_m = segLen_m - ts.posOnSegment_m;
		if (remaining_m < 0.0)
			remaining_m = 0.0;
		double speed = std::max(ts.speed_ms, MIN_SPEED_FOR_ETA_MS);
		double clearSec = remaining_m / speed;

		std::string key = p[ts.segmentIndex]->getName() + "->"
						  + p[ts.segmentIndex + 1]->getName();
		/* Multiple occupants on one segment: a new entrant has to wait
		   for the LAST one to clear, so keep the max. */
		auto it = clearTimes.find(key);
		if (it == clearTimes.end() || clearSec > it->second)
			clearTimes[key] = clearSec;
	}
	return clearTimes;
}

/* ---- Build expected event-delay map (event-aware routing) ---- */
SegmentEventRisk Simulation::buildEventRisk() const
{
	SegmentEventRisk risk;
	for (const auto &fromName : _network.getNodeNames())
	{
		for (const auto &edge : _network.getNeighbours(fromName))
		{
			const std::string &toName = edge.destination->getName();
			double expectedDelay = 0.0;

			/* Rail events bound to this physical segment (bidirectional
			   match), plus station events at the destination — arriving
			   there risks triggering them too. */
			for (const auto *ev : getEventsOnSegment(fromName, toName))
				expectedDelay += ev->getProbability() * ev->getDuration();
			for (const auto *ev : getEventsAtNode(toName))
				expectedDelay += ev->getProbability() * ev->getDuration();

			if (expectedDelay > 0.0)
				risk[fromName + "->" + toName] = expectedDelay;
		}
	}
	return risk;
}

/* ---- Check if any remaining segment on the path is congested ---- */
bool Simulation::hasCongestedSegmentAhead(
	const TrainState &s, const SegmentOccupancy &occupancy) const
{
	if (occupancy.empty())
		return false;
	const auto &path = s.train->getPath();
	for (size_t i = s.segmentIndex; i + 1 < path.size(); i++)
	{
		std::string key = path[i]->getName() + "->"
						  + path[i + 1]->getName();
		auto it = occupancy.find(key);
		if (it != occupancy.end() && it->second > 0)
			return true;
	}
	return false;
}

/* ---- Would this train actually be blocked by a slower train ahead? ---- */
bool Simulation::wouldBeBlocked(
	const TrainState &s, const std::vector<TrainState> &states) const
{
	const auto &path = s.train->getPath();
	if (path.size() < 2 || s.segmentIndex + 1 >= path.size())
		return false;

	double myAccel = s.train->getAccelRate();

	/* ETA to reach the start of a future segment, used below to skip
	   congestion that will have resolved before this train arrives.
	   This function always runs right after a segment transition, so
	   s.segmentIndex is the segment we just entered (posOnSegment_m
	   == 0) — ETA to reach ITS start is 0 by definition. */
	double etaAccum = 0.0;

	/* Scan upcoming segments for a train that would block us */
	for (size_t seg = s.segmentIndex; seg + 1 < path.size(); seg++)
	{
		const std::string &segFrom = path[seg]->getName();
		const std::string &segTo = path[seg + 1]->getName();

		for (const auto &other : states)
		{
			if (other.train == s.train || other.arrived || !other.departed)
				continue;
			const auto &op = other.train->getPath();
			if (op.size() < 2 || other.segmentIndex + 1 >= op.size())
				continue;
			if (op[other.segmentIndex]->getName() != segFrom
				|| op[other.segmentIndex + 1]->getName() != segTo)
				continue;

			/* Predictive check: if this segment will already be clear
			   by the time we'd actually reach it, don't treat it as
			   blocking — avoids wasted reroutes for congestion several
			   segments away that resolves long before we get there.
			   (For seg == s.segmentIndex, etaAccum reflects "now", so
			   an immediate real block is never suppressed here.) */
			double otherSegLen_m, otherSegSpd_ms;
			getSegmentInfo(segFrom, segTo, otherSegLen_m, otherSegSpd_ms);
			double otherRemaining_m = otherSegLen_m - other.posOnSegment_m;
			if (otherRemaining_m < 0.0)
				otherRemaining_m = 0.0;
			double otherClearSec = otherRemaining_m
				/ std::max(other.speed_ms, MIN_SPEED_FOR_ETA_MS);
			if (otherClearSec <= etaAccum)
				continue; // will be gone before we arrive

			/* Another train is on this segment. It blocks us if:
			   - it is ahead AND currently slower (blocking right now), OR
			   - it has a lower accel rate (will be slower after stops) */
			double otherAccel = other.train->getAccelRate();
			if (other.posOnSegment_m >= s.posOnSegment_m
				&& (other.speed_ms < s.speed_ms - 0.5
					|| otherAccel < myAccel * 0.95))
				return true;
		}

		/* Advance ETA by this segment's traversal time so the NEXT
		   iteration's etaAccum reflects "time to reach the start of
		   the following segment", not this one. Use our own current
		   speed for the segment we're on now, and the segment's speed
		   limit (optimistic cruise estimate) for segments further out. */
		double segLen_m, segSpd_ms;
		getSegmentInfo(segFrom, segTo, segLen_m, segSpd_ms);
		double travelSpeed = (seg == s.segmentIndex)
			? std::max(s.speed_ms, MIN_SPEED_FOR_ETA_MS)
			: segSpd_ms;
		etaAccum += segLen_m / travelSpeed;
	}
	return false;
}

/* ---- Compute congestion-weighted cost for a (sub)path ---- */
double Simulation::computePathCost(
	const std::vector<std::shared_ptr<Node>> &path,
	size_t startIdx,
	const SegmentOccupancy &occupancy,
	const SegmentClearTimes &clearTimes,
	const SegmentEventRisk &eventRisk) const
{
	double cost = 0.0;
	for (size_t i = startIdx; i + 1 < path.size(); i++)
	{
		const std::string &from = path[i]->getName();
		const std::string &to = path[i + 1]->getName();
		for (const auto &edge : _network.getNeighbours(from))
		{
			if (edge.destination->getName() == to)
			{
				cost += DijkstraPathfinding::edgeWeight(
					edge, _weightMode, from, occupancy, clearTimes, eventRisk);
				break;
			}
		}
	}
	return cost;
}

/* ---- Re-route a train from its current node using occupancy data ---- */
void Simulation::rerouteFromNode(TrainState &s,
								 SegmentOccupancy &occupancy,
								 SegmentClearTimes &clearTimes,
								 const SegmentEventRisk &eventRisk)
{
	const auto &path = s.train->getPath();
	if (s.segmentIndex >= path.size())
		return;

	const std::string &currentNode = path[s.segmentIndex]->getName();
	const std::string &dest = s.train->getArrivalStation();
	if (currentNode == dest)
		return;

	std::vector<std::shared_ptr<Node>> newTail;
	try
	{
		newTail = _pathfinder->findPath(currentNode, dest, _network,
										_weightMode, occupancy, clearTimes,
										eventRisk);
	}
	catch (...)
	{
		return; // keep current path on failure
	}
	if (newTail.size() < 2)
		return;

	/* Only apply if the new tail differs from the current remaining path */
	bool same = (newTail.size() == path.size() - s.segmentIndex);
	if (same)
	{
		for (size_t i = 0; i < newTail.size(); i++)
		{
			if (newTail[i]->getName()
				!= path[s.segmentIndex + i]->getName())
			{
				same = false;
				break;
			}
		}
	}
	if (same)
		return;

	/* Cost-margin guard: only reroute if the new path is meaningfully
	   cheaper in congestion-weighted terms.  This prevents marginal
	   reroutes where the alt corridor has become almost as congested
	   as the main one (e.g. FastF after FastB and FastD already moved). */
	static constexpr double REROUTE_MARGIN = 0.10; // 10 % improvement needed
	double oldCost = computePathCost(path, s.segmentIndex, occupancy,
									 clearTimes, eventRisk);
	double newCost = computePathCost(newTail, 0, occupancy, clearTimes,
									  eventRisk);
	if (newCost >= oldCost * (1.0 - REROUTE_MARGIN))
		return;

	/* Old next segment (the one this train would have taken) is no
	   longer occupied by it — captured before the path is replaced. */
	std::string oldKey;
	if (s.segmentIndex + 1 < path.size())
		oldKey = path[s.segmentIndex]->getName() + "->"
				+ path[s.segmentIndex + 1]->getName();

	/* Build the full path: segments already traversed + new tail */
	std::vector<std::shared_ptr<Node>> newPath;
	for (size_t i = 0; i < s.segmentIndex; i++)
		newPath.push_back(path[i]);
	for (auto &n : newTail)
		newPath.push_back(n);

	s.train->setPath(newPath);
	s.segmentIndex = newPath.size() - newTail.size();
	s.train->setPathIndex(s.segmentIndex);
	s.segsSinceReroute = 0;

	/* Update the shared tick-local occupancy/clear-time maps so any
	   train evaluated later in this same tick sees this train's new
	   choice, instead of a stale tick-start snapshot (avoids several
	   trains piling onto the same "empty" alternate corridor at once). */
	if (!oldKey.empty())
	{
		auto it = occupancy.find(oldKey);
		if (it != occupancy.end())
		{
			it->second--;
			if (it->second <= 0)
			{
				occupancy.erase(it);
				clearTimes.erase(oldKey);
			}
		}
	}
	if (s.segmentIndex + 1 < newPath.size())
	{
		std::string newKey = newPath[s.segmentIndex]->getName() + "->"
							 + newPath[s.segmentIndex + 1]->getName();
		occupancy[newKey]++;
		double segLen_m, segSpd_ms;
		getSegmentInfo(newPath[s.segmentIndex]->getName(),
					   newPath[s.segmentIndex + 1]->getName(),
					   segLen_m, segSpd_ms);
		double selfClearSec = segLen_m
			/ std::max(s.speed_ms, MIN_SPEED_FOR_ETA_MS);
		auto it = clearTimes.find(newKey);
		if (it == clearTimes.end() || selfClearSec > it->second)
			clearTimes[newKey] = selfClearSec;
	}

	/* Log the reroute */
	if (!_quiet)
	{
		std::string newRoute;
		for (size_t i = s.segmentIndex; i < newPath.size(); i++)
		{
			if (i > s.segmentIndex)
				newRoute += " -> ";
			newRoute += newPath[i]->getName();
		}
		_output.printReroute(*s.train, newRoute);
	}
}

/* ---- Overtaking / blocking between trains ---- */
void Simulation::applyBlocking(std::vector<TrainState> &states)
{
	/* For each pair of trains on the same segment, if one is behind
	   and faster, cap its speed to the one ahead's speed. */
	for (size_t i = 0; i < states.size(); i++)
	{
		if (states[i].arrived || !states[i].departed)
			continue;
		auto &pi = states[i].train->getPath();
		if (pi.size() < 2 || states[i].segmentIndex + 1 >= pi.size())
			continue;

		const std::string &fromI =
			pi[states[i].segmentIndex]->getName();
		const std::string &toI =
			pi[states[i].segmentIndex + 1]->getName();

		for (size_t j = i + 1; j < states.size(); j++)
		{
			if (states[j].arrived || !states[j].departed)
				continue;
			auto &pj = states[j].train->getPath();
			if (pj.size() < 2 || states[j].segmentIndex + 1 >= pj.size())
				continue;

			const std::string &fromJ =
				pj[states[j].segmentIndex]->getName();
			const std::string &toJ =
				pj[states[j].segmentIndex + 1]->getName();

			/* Same segment? */
			if (fromI != fromJ || toI != toJ)
				continue;

			/* Determine who is ahead / behind */
			TrainState *ahead = &states[i];
			TrainState *behind = &states[j];
			if (states[j].posOnSegment_m > states[i].posOnSegment_m)
			{
				ahead = &states[j];
				behind = &states[i];
			}

			/* If behind is faster, cap its speed */
			if (behind->speed_ms > ahead->speed_ms)
			{
				behind->speed_ms = ahead->speed_ms;
				behind->train->setStatus(TrainStatus::Delayed);
			}
		}
	}
}

/* ---- Events at a node ---- */
std::vector<const Event *> Simulation::getEventsAtNode(
	const std::string &nodeName) const
{
	std::vector<const Event *> result;
	for (const auto &event : _events)
	{
		if (!event.isRailEvent() && event.getNodeName() == nodeName)
			result.push_back(&event);
	}
	return result;
}

/* ---- Events on a rail segment ---- */
std::vector<const Event *> Simulation::getEventsOnSegment(
	const std::string &from, const std::string &to) const
{
	std::vector<const Event *> result;
	for (const auto &event : _events)
	{
		if (!event.isRailEvent())
			continue;
		/* Bidirectional match */
		if ((event.getNodeName() == from && event.getNodeName2() == to)
			|| (event.getNodeName() == to
				&& event.getNodeName2() == from))
			result.push_back(&event);
	}
	return result;
}
