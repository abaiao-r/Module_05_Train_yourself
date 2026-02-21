/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Train.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRAIN_HPP
#define TRAIN_HPP

#include <string>

class Train
{
  private:
	std::string _name;
	double _maxAcceleration;
	double _maxBrakingForce;
	std::string _departureStation;
	std::string _arrivalStation;
	std::string _departureTime;
	double _currentSpeed;
	std::string _currentStation;

  public:
	Train(const std::string &name, double maxAcceleration,
		  double maxBrakingForce, const std::string &departure,
		  const std::string &arrival, const std::string &departureTime);
	Train(const Train &src);
	Train &operator=(const Train &src);
	~Train();

	const std::string &getName() const;
	double getMaxAcceleration() const;
	double getMaxBrakingForce() const;
	const std::string &getDepartureStation() const;
	const std::string &getArrivalStation() const;
	const std::string &getDepartureTime() const;
	double getCurrentSpeed() const;
	const std::string &getCurrentStation() const;
};

#endif
