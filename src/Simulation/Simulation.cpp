/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simulation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/22 13:05:50 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Simulation.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <utility>

#include "Node.hpp"

/* ---- Constructor / Destructor ---- */
Simulation::Simulation(RailNetwork network,
					   std::vector<std::unique_ptr<Train>> trains,
					   std::vector<Event> events,
					   std::unique_ptr<IPathfinding> pathfinder,
					   PathWeightMode weightMode)
	: _network(std::move(network)), _trains(std::move(trains)),
	  _events(std::move(events)), _pathfinder(std::move(pathfinder)),
	  _weightMode(weightMode), _rng(std::random_device{}())
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

/* ---- Public ---- */
void Simulation::run()
{
	_output.printNetwork(_network);
	computePaths();

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
					handleSegmentTransition(s, i);
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
	}

	/* Finalize observer files */
	for (size_t i = 0; i < states.size(); i++)
	{
		_observers[i]->onTrainFinish(states[i].train->getName(),
									 states[i].timeSinceDepart);
	}

	_output.printResult(_trains);
}

/* ---- Path computation ---- */
void Simulation::computePaths()
{
	std::cout << "=== Paths ===" << std::endl;
	for (auto &train : _trains)
	{
		std::vector<std::shared_ptr<Node>> path;
		try
		{
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
		_output.printTrainPath(*train);
	}
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

		/* Time to accelerate from 0 to speed limit */
		double tAccel = speedLim_ms / accel;
		double dAccel = 0.5 * accel * tAccel * tAccel;

		/* Time to brake from speed limit to 0 */
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
void Simulation::handleSegmentTransition(TrainState &s, size_t trainIdx)
{
	auto &path = s.train->getPath();
	if (path.size() < 2)
		return;
	bool isLast = (s.segmentIndex + 2 >= path.size());

	/* Notify arrival at next node */
	std::string arrNode = path[s.segmentIndex + 1]->getName();
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
