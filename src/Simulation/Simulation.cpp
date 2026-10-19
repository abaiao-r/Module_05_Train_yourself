/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simulation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/06 12:18:34 by ctw03933         ###   ########.fr       */
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

const std::vector<Event> &Simulation::getEvents() const
{
	return _events;
}

void Simulation::setAnimCallback(AnimTickCallback cb)
{
	_animCallback = std::move(cb);
}

void Simulation::setEventTriggerCallback(EventTriggerCallback cb)
{
	_eventTriggerCallback = std::move(cb);
}

void Simulation::setRerouteCallback(RerouteCallback cb)
{
	_rerouteCallback = std::move(cb);
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
	_topologyGen++;
}

void Simulation::applyAddRail(const AddRailCommand &c)
{
	_network.addConnection(c.from, c.to, c.distanceKm, c.speedLimitKmh);
	_topologyGen++;
}

void Simulation::applyAddEvent(const AddEventCommand &c)
{
	_network.findNode(c.node1);
	if (!c.node2.empty())
		_network.findNode(c.node2);
	_events.emplace_back(c.name, c.probability, c.durationSeconds,
						 c.node1, c.node2);
	_eventGen++;
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
		s.segmentEntryTime = train->getDepartureTime();
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
					s.segmentEntryTime = simTime;
					s.profileDirty = true;
					s.train->setStatus(TrainStatus::Running);
					if (!_quiet)
						_output.printDeparture(*s.train);
					rollSegmentEvents(s, i);
				}
				continue;
			}

			/* Station stop countdown */
			bool isStopped = false;
			if (s.stopTimer > 0.0)
			{
				s.stopTimer -= DT;
				if (s.stopTimer <= 0.0)
				{
					s.stopTimer = 0.0;
					s.profileDirty = true;
					/* Back in the queue: a train that overtook during
					   the dwell is now ahead, not behind. */
					s.segmentEntryTime = simTime;
					s.evalEntryDone = s.evalEventDone
						= s.evalPrePivotDone = false;
				}
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
					handleSegmentTransition(s, i, simTime);
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

		/* One snapshot of who is on which segment, shared by the
		   reroute rule and the blocking rule. */
		SegmentState segState = buildSegmentState(states);

		if (_weightMode == PathWeightMode::Adaptive)
		{
			for (const auto &kv : segState)
				for (size_t k = 1; k < kv.second.size(); k++)
					considerReroute(states, kv.second[k], kv.second, k,
									simTime);
		}

		/* Overtaking / blocking check */
		applyBlocking(states, segState, simTime);

		if (_animCallback)
		{
			for (auto &st : states)
				if (st.profileDirty && st.departed && !st.arrived)
					rebuildProfile(st, simTime);
		}

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
	physicsStep(path, s.segmentIndex, s.posOnSegment_m, s.speed_ms,
				s.train->getAccelRate(), s.train->getDecelRate());
}

void Simulation::physicsStep(const std::vector<std::shared_ptr<Node>> &path,
							 size_t segIdx, double &pos, double &speed,
							 double accel, double decel) const
{
	double segLen_m, speedLim_ms;
	getSegmentInfo(path[segIdx]->getName(), path[segIdx + 1]->getName(),
				   segLen_m, speedLim_ms);

	double distToEnd = segLen_m - pos;

	/* Determine target end speed: 0 at last segment, else next limit */
	double targetEnd = 0.0;
	bool isLast = (segIdx == path.size() - 2);
	if (!isLast)
	{
		double nLen, nSpd;
		getSegmentInfo(path[segIdx + 1]->getName(),
					   path[segIdx + 2]->getName(), nLen, nSpd);
		targetEnd = nSpd;
		if (targetEnd > speedLim_ms)
			targetEnd = speedLim_ms;
	}

	/* Braking distance needed to reach targetEnd */
	double dv = speed - targetEnd;
	double brakeDist = 0.0;
	if (dv > 0.0)
		brakeDist = (speed * speed - targetEnd * targetEnd) / (2.0 * decel);

	double newSpeed = speed;
	if (distToEnd <= brakeDist + 2.0 && speed > targetEnd + 0.1)
	{
		newSpeed = speed - decel * DT;
		if (newSpeed < targetEnd)
			newSpeed = targetEnd;
		if (newSpeed < 0.0)
			newSpeed = 0.0;
	}
	else if (speed < speedLim_ms - 0.1)
	{
		newSpeed = speed + accel * DT;
		if (newSpeed > speedLim_ms)
			newSpeed = speedLim_ms;
	}
	else
		newSpeed = speedLim_ms;

	pos += (speed + newSpeed) / 2.0 * DT;
	speed = newSpeed;
	if (pos > segLen_m)
		pos = segLen_m;
}

/* ---- Segment transition ---- */
void Simulation::handleSegmentTransition(TrainState &s, size_t trainIdx,
										 double simTime)
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
			if (_eventTriggerCallback)
				_eventTriggerCallback(s.train->getName(), event->getName(),
									  event->getNodeName(), delay);
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
		s.segmentEntryTime = simTime;
		s.evalEntryDone = s.evalEventDone = s.evalPrePivotDone = false;
		s.profileDirty = true;
		s.train->setPathIndex(s.segmentIndex);

		rollSegmentEvents(s, trainIdx);

		/* Apply stop duration at intermediate stations. The train is
		   stationary during the dwell, so its speed must reflect that -
		   otherwise it retains the previous segment's cruise speed and
		   never registers as an obstruction for blocking/reroute checks. */
		s.stopTimer = s.train->getStopDuration();
		if (s.stopTimer > 0.0)
			s.speed_ms = 0.0;

		/* Reroute cooldown bookkeeping. */
		s.segsSinceReroute++;
	}
}


/* Rail events on the segment the train is currently entering. */
void Simulation::rollSegmentEvents(TrainState &s, size_t trainIdx)
{
	const auto &path = s.train->getPath();
	if (s.segmentIndex + 1 >= path.size())
		return;
	std::string segFrom = path[s.segmentIndex]->getName();
	std::string segTo = path[s.segmentIndex + 1]->getName();
	for (const auto *event : getEventsOnSegment(segFrom, segTo))
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
			if (_eventTriggerCallback)
				_eventTriggerCallback(s.train->getName(),
									  event->getName(), loc, delay);
		}
	}
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

/* Risk only changes on live topology/event mutations, so reuse it. */
const SegmentEventRisk &Simulation::cachedEventRisk() const
{
	unsigned gen = _topologyGen * 65537u + _eventGen;
	if (_riskCacheGen != gen)
	{
		_riskCache = buildEventRisk();
		_riskCacheGen = gen;
	}
	return _riskCache;
}

/* ---- Exact kinematic ETA - deterministic, not an approximation.
   Uses the train's real accel capability, current speed and the
   segment's speed limit to solve for how long it actually takes to
   cover the remaining distance, whether or not it reaches cruise
   speed within that distance. ---- */
double Simulation::kinematicEta(double currentSpeed_ms, double accel_ms2,
								double remainingDist_m,
								double speedLimit_ms) const
{
	if (remainingDist_m <= 0.0)
		return 0.0;
	if (currentSpeed_ms >= speedLimit_ms || accel_ms2 <= 0.0)
		return remainingDist_m / std::max(speedLimit_ms, MIN_SPEED_FOR_ETA_MS);

	double timeToLimit = (speedLimit_ms - currentSpeed_ms) / accel_ms2;
	double distToLimit = currentSpeed_ms * timeToLimit
		+ 0.5 * accel_ms2 * timeToLimit * timeToLimit;

	if (distToLimit >= remainingDist_m)
	{
		/* Never reaches the limit within the remaining distance -
		   solve remainingDist = v0*t + 0.5*a*t^2 for t directly. */
		double a = 0.5 * accel_ms2;
		double b = currentSpeed_ms;
		double c = -remainingDist_m;
		double disc = b * b - 4.0 * a * c;
		if (disc < 0.0)
			return remainingDist_m
				/ std::max(currentSpeed_ms, MIN_SPEED_FOR_ETA_MS);
		return (-b + std::sqrt(disc)) / (2.0 * a);
	}

	double remainAfterLimit = remainingDist_m - distToLimit;
	return timeToLimit + remainAfterLimit / speedLimit_ms;
}

/* ---- Virtual replay of the real physics (pure, non-mutating) ---- */
Simulation::VTrain Simulation::makeVTrain(const TrainState &s,
										  double simTime) const
{
	VTrain v;
	v.path = &s.train->getPath();
	v.seg = s.segmentIndex;
	v.pos = s.posOnSegment_m;
	v.speed = s.speed_ms;
	v.stopTimer = s.stopTimer;
	v.accel = s.train->getAccelRate();
	v.decel = s.train->getDecelRate();
	v.stopDur = s.train->getStopDuration();
	v.entryT = s.segmentEntryTime - simTime;
	v.done = s.arrived;
	return v;
}

double Simulation::replayEta(VTrain me, const VTrain *leader,
							 std::vector<double> *nodeTimes) const
{
	if (me.path->size() < 2 || me.done)
		return 0.0;

	VTrain lead{};
	bool haveLeader = (leader != nullptr);
	if (haveLeader)
		lead = *leader;

	/* One virtual tick: dwell countdown, else physics plus node
	   transition (mirrors updatePhysics/handleSegmentTransition, minus
	   random events, which are deliberately excluded from ETAs). */
	auto advance = [this](VTrain &v, double now,
						  std::vector<double> *times) {
		if (v.done)
			return;
		if (v.stopTimer > 0.0)
		{
			v.stopTimer = std::max(0.0, v.stopTimer - DT);
			return;
		}
		physicsStep(*v.path, v.seg, v.pos, v.speed, v.accel, v.decel);
		double len, spd;
		getSegmentInfo((*v.path)[v.seg]->getName(),
					   (*v.path)[v.seg + 1]->getName(), len, spd);
		if (v.pos < len)
			return;
		if (times)
			times->push_back(now);
		if (v.seg + 2 >= v.path->size())
		{
			v.done = true;
			v.speed = 0.0;
			return;
		}
		v.seg++;
		v.pos = 0.0;
		v.entryT = now;
		v.stopTimer = v.stopDur;
		if (v.stopTimer > 0.0)
			v.speed = 0.0;
	};

	double t = 0.0;
	while (!me.done && t < REPLAY_MAX_SEC)
	{
		t += DT;
		/* Same order as the real loop: everyone moves, then blocking. */
		advance(me, t, nodeTimes);
		if (haveLeader)
			advance(lead, t, nullptr);

		if (haveLeader && !lead.done && !me.done && lead.stopTimer <= 0.0
			&& me.stopTimer <= 0.0
			&& (*me.path)[me.seg] == (*lead.path)[lead.seg]
			&& (*me.path)[me.seg + 1] == (*lead.path)[lead.seg + 1]
			&& lead.entryT <= me.entryT)
		{
			if (me.speed > lead.speed)
				me.speed = lead.speed;
			double maxPos = lead.pos - MIN_SAFE_FOLLOWING_DISTANCE_M;
			if (me.pos > maxPos)
				me.pos = std::max(maxPos, 0.0);
		}
	}
	return t;
}

/* ---- One fused pass: who is on which directed segment, in order ---- */
Simulation::SegmentState Simulation::buildSegmentState(
	const std::vector<TrainState> &states) const
{
	SegmentState seg;
	for (size_t i = 0; i < states.size(); i++)
	{
		const auto &ts = states[i];
		/* A train stopped at a node is not a blocker. */
		if (ts.arrived || !ts.departed || ts.stopTimer > 0.0)
			continue;
		const auto &p = ts.train->getPath();
		if (p.size() < 2 || ts.segmentIndex + 1 >= p.size())
			continue;
		seg[p[ts.segmentIndex]->getName() + "->"
			+ p[ts.segmentIndex + 1]->getName()]
			.push_back(i);
	}
	/* Right of way: earliest entry first, then stable index. The vector
	   is already index-ordered, so a stable sort keeps the tie-break. */
	for (auto &kv : seg)
	{
		std::stable_sort(kv.second.begin(), kv.second.end(),
						 [&](size_t a, size_t b) {
							 return states[a].segmentEntryTime
									< states[b].segmentEntryTime;
						 });
	}
	return seg;
}

/* ---- Profile: free-flow arrival time at each upcoming node ---- */
void Simulation::rebuildProfile(TrainState &s, double simTime)
{
	s.etaProfile.clear();
	s.profileDirty = false;
	s.lastProfileTime = simTime;
	if (s.arrived || !s.departed)
		return;
	const auto &p = s.train->getPath();
	if (p.size() < 2 || s.segmentIndex + 1 >= p.size())
		return;
	std::vector<double> times;
	replayEta(makeVTrain(s, simTime), nullptr, &times);
	for (double t : times)
		s.etaProfile.push_back(simTime + t);
}

/* ---- Cheap predicate every tick; expensive part only when triggered ----
   Rule: another train is on MY segment, it is AHEAD of me, I am FASTER,
   and its route keeps sharing my route after the next node. */
void Simulation::considerReroute(std::vector<TrainState> &states, size_t idx,
								 const std::vector<size_t> &group,
								 size_t posInGroup, double simTime)
{
	TrainState &s = states[idx];
	if (posInGroup == 0 || s.segsSinceReroute < REROUTE_COOLDOWN)
		return;
	const auto &path = s.train->getPath();
	if (s.segmentIndex + 2 >= path.size())
		return; // next node is the destination: no choice left

	/* Nearest train ahead whose route keeps sharing mine after the pivot
	   (a leader that diverges is skipped in favour of one further up). */
	const TrainState *leaderPtr = nullptr;
	size_t shared = 0;
	for (size_t g = posInGroup; g-- > 0 && !leaderPtr;)
	{
		const TrainState &cand = states[group[g]];
		const auto &lp = cand.train->getPath();
		size_t n = 0;
		for (size_t i = s.segmentIndex + 1, j = cand.segmentIndex + 1;
			 i + 1 < path.size() && j + 1 < lp.size(); i++, j++)
		{
			if (path[i] != lp[j] || path[i + 1] != lp[j + 1])
				break;
			n++;
		}
		if (n > 0)
		{
			leaderPtr = &cand;
			shared = n;
		}
	}
	if (!leaderPtr)
		return; // nobody ahead keeps blocking after the pivot
	const TrainState &leader = *leaderPtr;

	/* Am I faster? Free ETA to the segment end, both from current state. */
	double segLen_m, segSpd_ms;
	getSegmentInfo(path[s.segmentIndex]->getName(),
				   path[s.segmentIndex + 1]->getName(), segLen_m, segSpd_ms);
	double myEta = kinematicEta(s.speed_ms, s.train->getAccelRate(),
								segLen_m - s.posOnSegment_m, segSpd_ms);
	double leaderEta = kinematicEta(
		leader.speed_ms, leader.train->getAccelRate(),
		segLen_m - leader.posOnSegment_m, segSpd_ms);
	if (myEta >= leaderEta)
		return;

	/* Evaluation triggers, with a per-segment budget. */
	unsigned gen = _topologyGen * 65537u + _eventGen;
	bool leaderChanged = (s.lastLeader != leader.train
						  || s.lastLeaderVersion != leader.routeVersion
						  || s.lastGen != gen);
	bool nearPivot = (myEta <= PRE_PIVOT_HORIZON_SEC);
	bool run = false;
	if (!s.evalEntryDone)
	{
		s.evalEntryDone = true;
		if (nearPivot)
			s.evalPrePivotDone = true;
		run = true;
	}
	else if (leaderChanged && !s.evalEventDone)
	{
		s.evalEventDone = true;
		run = true;
	}
	else if (nearPivot && !s.evalPrePivotDone)
	{
		s.evalPrePivotDone = true;
		run = true;
	}
	if (!run)
		return;

	s.lastLeader = leader.train;
	s.lastLeaderVersion = leader.routeVersion;
	s.lastGen = gen;
	evaluateDetour(s, leader, shared, simTime);
}

bool Simulation::evaluateDetour(TrainState &s, const TrainState &leader,
								size_t sharedLen, double simTime)
{
	const auto &path = s.train->getPath();
	const std::string &pivot = path[s.segmentIndex + 1]->getName();
	const std::string &dest = s.train->getArrivalStation();

	ForbiddenEdges forbidden;
	std::string forbidKey;
	for (size_t i = 0; i < sharedLen; i++)
	{
		std::string k = path[s.segmentIndex + 1 + i]->getName() + "->"
						+ path[s.segmentIndex + 2 + i]->getName();
		forbidden.insert(k);
		forbidKey += k + ";";
	}

	/* Candidate-path cache: depends only on topology/events/dwell. */
	unsigned gen = _topologyGen * 65537u + _eventGen;
	if (_cacheGen != gen)
	{
		_candidateCache.clear();
		_cacheGen = gen;
	}
	double dwell = s.train->getStopDuration();
	std::string key = pivot + "|" + dest + "|" + forbidKey + "|"
					  + std::to_string(dwell);
	auto it = _candidateCache.find(key);
	if (it == _candidateCache.end())
	{
		std::vector<std::string> names;
		try
		{
			auto cand = _pathfinder->findConstrainedPath(
				pivot, dest, _network, forbidden, cachedEventRisk(), dwell);
			for (const auto &n : cand)
				names.push_back(n->getName());
		}
		catch (...)
		{
			names.clear();
		}
		it = _candidateCache.emplace(key, std::move(names)).first;
	}
	const auto &tail = it->second;
	if (tail.size() < 2)
		return false;

	/* Candidate identical to the current remainder: nothing to gain. */
	bool same = (tail.size() == path.size() - (s.segmentIndex + 1));
	for (size_t i = 0; same && i < tail.size(); i++)
		same = (tail[i] == path[s.segmentIndex + 1 + i]->getName());
	if (same)
		return false;

	std::vector<std::shared_ptr<Node>> newPath;
	for (size_t i = 0; i <= s.segmentIndex + 1; i++)
		newPath.push_back(path[i]);
	try
	{
		for (size_t i = 1; i < tail.size(); i++)
			newPath.push_back(_network.findNode(tail[i]));
	}
	catch (...)
	{
		return false;
	}

	/* The judge: identical replay for both options, leader included. */
	VTrain lead = makeVTrain(leader, simTime);
	VTrain stay = makeVTrain(s, simTime);
	/* Random events are not replayed, so their expected delay is added
	   per remaining segment to keep the two options comparable. */
	const SegmentEventRisk &risk = cachedEventRisk();
	auto riskOf = [&](const std::vector<std::shared_ptr<Node>> &p) {
		double sum = 0.0;
		for (size_t i = s.segmentIndex; i + 1 < p.size(); i++)
		{
			auto r = risk.find(p[i]->getName() + "->" + p[i + 1]->getName());
			if (r != risk.end())
				sum += r->second;
		}
		return sum;
	};
	double etaStay = replayEta(stay, &lead) + riskOf(path);
	VTrain det = stay;
	det.path = &newPath;
	double etaDetour = replayEta(det, &lead) + riskOf(newPath);
	if (!(etaDetour + MIN_GAIN_SEC < etaStay))
		return false;

	std::string newRoute;
	for (size_t i = s.segmentIndex; i < newPath.size(); i++)
	{
		if (i > s.segmentIndex)
			newRoute += " -> ";
		newRoute += newPath[i]->getName();
	}
	s.train->setPath(newPath);
	s.train->setPathIndex(s.segmentIndex);
	s.segsSinceReroute = 0;
	s.routeVersion++;
	s.profileDirty = true;
	s.rerouteCount++;
	s.lastRerouteReason = "stuck behind " + leader.train->getName()
		+ " (stay ~" + std::to_string(static_cast<int>(etaStay))
		+ "s vs detour ~" + std::to_string(static_cast<int>(etaDetour))
		+ "s)";
	if (!_quiet)
		_output.printReroute(*s.train, newRoute);
	if (_rerouteCallback)
		_rerouteCallback(s.train->getName(), pivot, newRoute);
	return true;
}

/* ---- Overtaking / blocking between trains ---- */
void Simulation::applyBlocking(std::vector<TrainState> &states,
							   const SegmentState &segState, double simTime)
{
	/* Delayed is a per-tick fact, not a permanent one - clear it up
	   front so a train that is no longer capped stops reporting it. */
	for (auto &st : states)
	{
		if (!st.arrived && st.departed
			&& st.train->getStatus() == TrainStatus::Delayed)
			st.train->setStatus(TrainStatus::Running);
		st.blockedByTrain = nullptr;
	}

	/* Right-of-way is the group order (entry order, which never flips -
	   raw position does not: a follower's own acceleration can drift it
	   numerically past the leader). The follower is capped in speed and
	   hard-clamped to a safe following distance behind each train ahead. */
	for (const auto &kv : segState)
	{
		const auto &g = kv.second;
		for (size_t a = 0; a < g.size(); a++)
		{
			for (size_t b = a + 1; b < g.size(); b++)
			{
				TrainState *ahead = &states[g[a]];
				TrainState *behind = &states[g[b]];
				bool capped = false;
				if (behind->speed_ms > ahead->speed_ms)
				{
					behind->speed_ms = ahead->speed_ms;
					capped = true;
				}
				double maxBehindPos =
					ahead->posOnSegment_m - MIN_SAFE_FOLLOWING_DISTANCE_M;
				if (behind->posOnSegment_m > maxBehindPos)
				{
					behind->posOnSegment_m = std::max(maxBehindPos, 0.0);
					capped = true;
				}
				if (capped)
				{
					behind->train->setStatus(TrainStatus::Delayed);
					behind->blockedByTrain = ahead->train;
					if (simTime - behind->lastProfileTime
						>= PROFILE_REBUILD_MIN_SEC)
						behind->profileDirty = true;
				}
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
