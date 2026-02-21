/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Event.hpp"

Event::Event(const std::string &name, double probability,
			 const std::string &duration, const std::string &node)
	: _name(name), _probability(probability), _duration(duration), _node(node)
{
}

Event::Event(const Event &src) { *this = src; }

Event &Event::operator=(const Event &src)
{
	if (this != &src)
	{
		_name = src._name;
		_probability = src._probability;
		_duration = src._duration;
		_node = src._node;
	}
	return *this;
}

Event::~Event() {}

const std::string &Event::getName() const { return _name; }
double Event::getProbability() const { return _probability; }
const std::string &Event::getDuration() const { return _duration; }
const std::string &Event::getNode() const { return _node; }
