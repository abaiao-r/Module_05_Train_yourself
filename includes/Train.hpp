/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Train.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 13:51:45 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/20 11:34:06 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRAIN_HPP
#define TRAIN_HPP

#include "libraries.hpp"

class Train
{
	private:
		std::string _name;
		int			_currentSpeed;
		int			_maxAcceleration;
		int			_maxBrakingForce;
		std::string _DepartureStation;
		std::string _ArrivalStation;
		std::string _currentStation;
		std::string _nextStation;
		// chrono::time_point<chrono::system_clock> _departureTime;
		// chrono::time_point<chrono::system_clock> _arrivalTime;
		// chrono::time_point<chrono::system_clock> _currentTime;

	public:
		Train();
		Train(const Train &src);
		~Train();
		Train &operator=(const Train &src);

		// Getters
		const std::string &getName() const;
		const int		  &getCurrentSpeed() const;
		const int		  &getMaxAcceleration() const;
		const int		  &getMaxBrakingForce() const;
		const std::string &getDepartureStation() const;
		const std::string &getArrivalStation() const;
		const std::string &getCurrentStation() const;
		// const chrono::time_point<chrono::system_clock> &getDepartureTime()
		// const;

		// methods
		void GoToStation(std::string station);
};

#endif
