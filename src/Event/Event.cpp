/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Event.hpp"

Event::Event(const std::string &name, double probability, double duration,
			 const std::string &nodeName, const std::string &nodeName2)
	: _name(name), _probability(probability), _duration(duration),
	  _nodeName(nodeName), _nodeName2(nodeName2)
{
	if (name.empty())
		throw std::invalid_argument("Event name cannot be empty");
	if (probability < 0.0 || probability > 1.0)
		throw std::invalid_argument("Probability must be in [0, 1]");
	if (duration < 0.0)
		throw std::invalid_argument("Duration cannot be negative");
	if (nodeName.empty())
		throw std::invalid_argument("Node name cannot be empty");
}

Event::Event(const Event &src)
	: _name(src._name), _probability(src._probability),
	  _duration(src._duration), _nodeName(src._nodeName),
	  _nodeName2(src._nodeName2)
{
}

Event &Event::operator=(const Event &src)
{
	if (this != &src)
	{
		_name = src._name;
		_probability = src._probability;
		_duration = src._duration;
		_nodeName = src._nodeName;
		_nodeName2 = src._nodeName2;
	}
	return *this;
}

Event::~Event() {}

bool Event::tryTrigger(std::mt19937 &rng) const
{
	if (_probability <= 0.0)
		return false;
	std::uniform_real_distribution<double> dist(0.0, 1.0);
	return dist(rng) < _probability;
}

const std::string &Event::getName() const { return _name; }
double Event::getProbability() const { return _probability; }
double Event::getDuration() const { return _duration; }
const std::string &Event::getNodeName() const { return _nodeName; }
const std::string &Event::getNodeName2() const { return _nodeName2; }
bool Event::isRailEvent() const { return !_nodeName2.empty(); }
