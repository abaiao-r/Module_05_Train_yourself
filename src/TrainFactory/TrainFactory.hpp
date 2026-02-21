/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TrainFactory.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRAINFACTORY_HPP
#define TRAINFACTORY_HPP

#include <memory>
#include <string>

#include "Train.hpp"

class TrainFactory
{
  public:
	TrainFactory();
	TrainFactory(const TrainFactory &src);
	TrainFactory &operator=(const TrainFactory &src);
	~TrainFactory();

	static std::unique_ptr<Train> createTrain(
		const std::string &name, double maxAcceleration,
		double maxBrakingForce, const std::string &departure,
		const std::string &arrival, const std::string &departureTime);
};

#endif
