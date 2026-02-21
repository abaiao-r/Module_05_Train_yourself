/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Train.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Train.hpp"

#include "Node.hpp"

/* ---- Canonical form ---- */
Train::Train(const std::string &name, double acceleration, double braking,
			 const std::string &departure, const std::string &arrival,
			 double departureTime)
	: _name(name), _maxAcceleration(acceleration),
	  _maxBrakingForce(braking), _departureStation(departure),
	  _arrivalStation(arrival), _departureTime(departureTime),
	  _status(TrainStatus::Waiting), _pathIndex(0),
	  _currentTime(departureTime), _totalDelay(0.0)
{
}

Train::Train(const Train &src)
	: _name(src._name), _maxAcceleration(src._maxAcceleration),
	  _maxBrakingForce(src._maxBrakingForce),
	  _departureStation(src._departureStation),
	  _arrivalStation(src._arrivalStation),
	  _departureTime(src._departureTime), _status(src._status),
	  _path(src._path), _pathIndex(src._pathIndex),
	  _currentTime(src._currentTime), _totalDelay(src._totalDelay)
{
}

Train &Train::operator=(const Train &src)
{
	if (this != &src)
	{
		_name = src._name;
		_maxAcceleration = src._maxAcceleration;
		_maxBrakingForce = src._maxBrakingForce;
		_departureStation = src._departureStation;
		_arrivalStation = src._arrivalStation;
		_departureTime = src._departureTime;
		_status = src._status;
		_path = src._path;
		_pathIndex = src._pathIndex;
		_currentTime = src._currentTime;
		_totalDelay = src._totalDelay;
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

/* ---- Getters ---- */
const std::string &Train::getName() const { return _name; }
double Train::getMaxAcceleration() const { return _maxAcceleration; }
double Train::getMaxBrakingForce() const { return _maxBrakingForce; }

const std::string &Train::getDepartureStation() const
{
	return _departureStation;
}

const std::string &Train::getArrivalStation() const
{
	return _arrivalStation;
}

double Train::getDepartureTime() const { return _departureTime; }
TrainStatus Train::getStatus() const { return _status; }

const std::vector<std::shared_ptr<Node>> &Train::getPath() const
{
	return _path;
}

size_t Train::getPathIndex() const { return _pathIndex; }
double Train::getCurrentTime() const { return _currentTime; }
double Train::getTotalDelay() const { return _totalDelay; }

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
