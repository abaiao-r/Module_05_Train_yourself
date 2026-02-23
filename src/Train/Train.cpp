/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Train.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Train.hpp"

#include "Node.hpp"

/* ---- Canonical form ---- */
Train::Train(int id, const std::string &name, double weight, double friction,
			 double accelForce, double brakeForce,
			 const std::string &departure, const std::string &arrival,
			 double departureTime, double stopDuration)
	: _id(id), _name(name), _weight(weight), _frictionCoefficient(friction),
	  _maxAccelForce(accelForce), _maxBrakeForce(brakeForce),
	  _departureStation(departure), _arrivalStation(arrival),
	  _departureTime(departureTime), _stopDuration(stopDuration),
	  _status(TrainStatus::Waiting), _pathIndex(0),
	  _currentTime(departureTime), _totalDelay(0.0),
	  _currentSpeed(0.0), _posOnSegment(0.0)
{
}

Train::Train(const Train &src)
	: _id(src._id), _name(src._name), _weight(src._weight),
	  _frictionCoefficient(src._frictionCoefficient),
	  _maxAccelForce(src._maxAccelForce),
	  _maxBrakeForce(src._maxBrakeForce),
	  _departureStation(src._departureStation),
	  _arrivalStation(src._arrivalStation),
	  _departureTime(src._departureTime),
	  _stopDuration(src._stopDuration), _status(src._status),
	  _path(src._path), _pathIndex(src._pathIndex),
	  _currentTime(src._currentTime), _totalDelay(src._totalDelay),
	  _currentSpeed(src._currentSpeed),
	  _posOnSegment(src._posOnSegment)
{
}

Train &Train::operator=(const Train &src)
{
	if (this != &src)
	{
		_id = src._id;
		_name = src._name;
		_weight = src._weight;
		_frictionCoefficient = src._frictionCoefficient;
		_maxAccelForce = src._maxAccelForce;
		_maxBrakeForce = src._maxBrakeForce;
		_departureStation = src._departureStation;
		_arrivalStation = src._arrivalStation;
		_departureTime = src._departureTime;
		_stopDuration = src._stopDuration;
		_status = src._status;
		_path = src._path;
		_pathIndex = src._pathIndex;
		_currentTime = src._currentTime;
		_totalDelay = src._totalDelay;
		_currentSpeed = src._currentSpeed;
		_posOnSegment = src._posOnSegment;
	}
	return *this;
}

Train::~Train() {}

/* ---- Mutators ---- */
void Train::setPath(const std::vector<std::shared_ptr<Node>> &path)
{
	_path = path;
	_pathIndex = 0;
}

void Train::advanceToNextNode(double travelTime)
{
	if (_path.size() < 2)
		return;
	_currentTime += travelTime;
	_pathIndex++;
	if (_pathIndex >= _path.size() - 1)
		_status = TrainStatus::Arrived;
	else
		_status = TrainStatus::Running;
}

void Train::applyDelay(double seconds)
{
	_totalDelay += seconds;
	_currentTime += seconds;
}

void Train::setStatus(TrainStatus status) { _status = status; }
void Train::setCurrentSpeed(double speed) { _currentSpeed = speed; }
void Train::setPosOnSegment(double pos) { _posOnSegment = pos; }
void Train::setPathIndex(size_t idx) { _pathIndex = idx; }
void Train::setCurrentTime(double t) { _currentTime = t; }

/* ---- Getters ---- */
int Train::getId() const { return _id; }
const std::string &Train::getName() const { return _name; }
double Train::getWeight() const { return _weight; }

double Train::getFrictionCoefficient() const
{
	return _frictionCoefficient;
}

double Train::getMaxAccelForce() const { return _maxAccelForce; }
double Train::getMaxBrakeForce() const { return _maxBrakeForce; }

const std::string &Train::getDepartureStation() const
{
	return _departureStation;
}

const std::string &Train::getArrivalStation() const
{
	return _arrivalStation;
}

double Train::getDepartureTime() const { return _departureTime; }
double Train::getStopDuration() const { return _stopDuration; }
TrainStatus Train::getStatus() const { return _status; }

const std::vector<std::shared_ptr<Node>> &Train::getPath() const
{
	return _path;
}

size_t Train::getPathIndex() const { return _pathIndex; }
double Train::getCurrentTime() const { return _currentTime; }
double Train::getTotalDelay() const { return _totalDelay; }
double Train::getCurrentSpeed() const { return _currentSpeed; }
double Train::getPosOnSegment() const { return _posOnSegment; }

std::string Train::getCurrentNodeName() const
{
	if (_path.empty())
		return "";
	return _path[_pathIndex]->getName();
}

bool Train::hasArrived() const
{
	return _status == TrainStatus::Arrived;
}

/* ---- Physics helpers ---- */
double Train::getAccelRate() const
{
	double mass_kg = _weight * 1000.0;
	double friction_N = _frictionCoefficient * mass_kg * 9.81;
	double force_N = _maxAccelForce * 1000.0;
	double net = force_N - friction_N;
	return (net > 0.0) ? net / mass_kg : 0.0;
}

double Train::getDecelRate() const
{
	double mass_kg = _weight * 1000.0;
	double friction_N = _frictionCoefficient * mass_kg * 9.81;
	double force_N = _maxBrakeForce * 1000.0;
	return (force_N + friction_N) / mass_kg;
}

std::string Train::statusToString(TrainStatus status)
{
	switch (status)
	{
		case TrainStatus::Waiting:
			return "Waiting";
		case TrainStatus::Running:
			return "Running";
		case TrainStatus::Arrived:
			return "Arrived";
		case TrainStatus::Delayed:
			return "Delayed";
	}
	return "Unknown";
}
