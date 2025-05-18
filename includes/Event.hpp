/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 12:14:08 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/18 20:13:52 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENT_HPP
#define EVENT_HPP

#include "libraries.hpp"

class Event
{
	private:
		std::string			 _eventName;
		float				 _eventProbability;
		std::chrono::seconds _eventDuration;

	public:
		Event() = delete;
		Event(const std::string &eventName, float probability,
			  std::chrono::seconds duration);
		Event(const Event &src) = delete;
		Event(Event &&src) = delete;
		~Event();
		Event &operator=(const Event &src) = delete;
		Event &operator=(Event &&src) = delete;

		const std::string		  &getEventName() const;
		const float				&getEventProbability() const;
		const std::chrono::seconds &getEventDuration() const;

		// Custom exception classes
		class InvalidEventNameException : public std::invalid_argument {
		public:
			InvalidEventNameException()
				: std::invalid_argument("Event name cannot be empty.") {}
		};
		class InvalidEventProbabilityException : public std::invalid_argument {
		public:
			InvalidEventProbabilityException()
				: std::invalid_argument("Event probability must be between 0.0 and 1.0.") {}
		};
		class InvalidEventDurationException : public std::invalid_argument {
		public:
			InvalidEventDurationException()
				: std::invalid_argument("Event duration must be positive.") {}
		};
};

#endif
