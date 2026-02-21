/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TrainFactory.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TrainFactory.hpp"

#include <stdexcept>

std::unique_ptr<Train> TrainFactory::createTrain(
	const std::string &name, double acceleration, double braking,
	const std::string &departure, const std::string &arrival,
	double departureTime)
{
	if (name.empty())
		throw std::invalid_argument("Train name cannot be empty");
	if (acceleration <= 0.0)
		throw std::invalid_argument("Acceleration must be positive");
	if (braking <= 0.0)
		throw std::invalid_argument("Braking force must be positive");
	if (departure.empty() || arrival.empty())
		throw std::invalid_argument("Station names cannot be empty");
	if (departure == arrival)
		throw std::invalid_argument("Departure and arrival must differ");
	return std::make_unique<Train>(name, acceleration, braking, departure,
								   arrival, departureTime);
}
