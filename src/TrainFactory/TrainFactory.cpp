/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TrainFactory.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TrainFactory.hpp"

TrainFactory::TrainFactory() {}
TrainFactory::TrainFactory(const TrainFactory &) {}
TrainFactory &TrainFactory::operator=(const TrainFactory &) { return *this; }
TrainFactory::~TrainFactory() {}

std::unique_ptr<Train> TrainFactory::createTrain(
	const std::string &name, double maxAcceleration, double maxBrakingForce,
	const std::string &departure, const std::string &arrival,
	const std::string &departureTime)
{
	return std::make_unique<Train>(name, maxAcceleration, maxBrakingForce,
								   departure, arrival, departureTime);
}
