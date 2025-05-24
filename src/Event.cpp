/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 18:28:53 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 12:40:59 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Event.hpp"

// Parameterized constructor
Event::Event(const std::string &eventName, float probability,
			 std::chrono::seconds duration,
			 const std::string   &eventLocalization)
	: _eventName(eventName),
	  _eventProbability(probability),
	  _eventDuration(duration),
	  _eventLocalization(eventLocalization)
{
	if (eventName.empty())
	{
		throw InvalidEventNameException();
	}
	if (probability < 0.0f || probability > 1.0f)
	{
		throw InvalidEventProbabilityException();
	}
	if (duration.count() <= 0)
	{
		throw InvalidEventDurationException();
	}
}

// Destructor
Event::~Event()
{
}

// Getters
const std::string &Event::getEventName() const
{
	return _eventName;
}

const float &Event::getEventProbability() const
{
	return _eventProbability;
}

const std::chrono::seconds &Event::getEventDuration() const
{
	return _eventDuration;
}

const std::string &Event::getEventLocalization() const
{
	return _eventLocalization;
}
