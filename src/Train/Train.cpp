/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Train.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Train.hpp"

Train::Train(const std::string &name, double maxAcceleration,
			 double maxBrakingForce, const std::string &departure,
			 const std::string &arrival, const std::string &departureTime)
	: _name(name),
	  _maxAcceleration(maxAcceleration),
	  _maxBrakingForce(maxBrakingForce),
	  _departureStation(departure),
	  _arrivalStation(arrival),
	  _departureTime(departureTime),
	  _currentSpeed(0.0),
	  _currentStation(departure)
{
}

Train::Train(const Train &src) { *this = src; }

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
		_currentSpeed = src._currentSpeed;
		_currentStation = src._currentStation;
	}
	return *this;
}

Train::~Train() {}

const std::string &Train::getName() const { return _name; }
double Train::getMaxAcceleration() const { return _maxAcceleration; }
double Train::getMaxBrakingForce() const { return _maxBrakingForce; }
const std::string &Train::getDepartureStation() const
{
	return _departureStation;
}
const std::string &Train::getArrivalStation() const { return _arrivalStation; }
const std::string &Train::getDepartureTime() const { return _departureTime; }
double Train::getCurrentSpeed() const { return _currentSpeed; }
const std::string &Train::getCurrentStation() const { return _currentStation; }
