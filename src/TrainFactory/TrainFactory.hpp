/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TrainFactory.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by ctw03933         ###   ########.fr       */
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
	TrainFactory() = delete;

	static std::unique_ptr<Train> createTrain(
		const std::string &name, double weight, double friction,
		double accelForce, double brakeForce,
		const std::string &departure, const std::string &arrival,
		double departureTime, double stopDuration);

	static void resetIdCounter();

  private:
	static int _nextId;
};

#endif
