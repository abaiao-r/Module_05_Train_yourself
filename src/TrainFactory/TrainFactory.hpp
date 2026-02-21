/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TrainFactory.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r         ###   ########.fr       */
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
		const std::string &name, double acceleration, double braking,
		const std::string &departure, const std::string &arrival,
		double departureTime);
};

#endif
