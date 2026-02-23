/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TrainFactory.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TrainFactory.hpp"

#include <stdexcept>

int TrainFactory::_nextId = 1;

void TrainFactory::resetIdCounter() { _nextId = 1; }

std::unique_ptr<Train> TrainFactory::createTrain(
	const std::string &name, double weight, double friction,
	double accelForce, double brakeForce,
	const std::string &departure, const std::string &arrival,
	double departureTime, double stopDuration)
{
	if (name.empty())
		throw std::invalid_argument("Train name cannot be empty");
	if (weight <= 0.0)
		throw std::invalid_argument("Weight must be positive");
	if (friction < 0.0)
		throw std::invalid_argument("Friction coefficient cannot be negative");
	if (accelForce <= 0.0)
		throw std::invalid_argument("Acceleration force must be positive");
	if (brakeForce <= 0.0)
		throw std::invalid_argument("Braking force must be positive");
	if (departure.empty() || arrival.empty())
		throw std::invalid_argument("Station names cannot be empty");
	if (departure == arrival)
		throw std::invalid_argument("Departure and arrival must differ");
	if (departureTime < 0.0)
		throw std::invalid_argument("Departure time cannot be negative");
	if (stopDuration < 0.0)
		throw std::invalid_argument("Stop duration cannot be negative");
	return std::make_unique<Train>(_nextId++, name, weight, friction,
								   accelForce, brakeForce, departure,
								   arrival, departureTime, stopDuration);
}
