/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventManager.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventManager.hpp"

#include <utility>

EventManager::EventManager() {}

EventManager::EventManager(const EventManager &src) { *this = src; }

EventManager &EventManager::operator=(const EventManager &src)
{
	if (this != &src)
		_events = src._events;
	return *this;
}

EventManager::EventManager(EventManager &&src) noexcept
{
	*this = std::move(src);
}

EventManager &EventManager::operator=(EventManager &&src) noexcept
{
	if (this != &src)
		_events = std::move(src._events);
	return *this;
}

EventManager::~EventManager() {}

void EventManager::addEvent(const Event &event) { _events.push_back(event); }

const std::vector<Event> &EventManager::getEvents() const { return _events; }
