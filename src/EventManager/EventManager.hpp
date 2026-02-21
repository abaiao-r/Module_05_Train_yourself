/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventManager.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTMANAGER_HPP
#define EVENTMANAGER_HPP

#include <vector>

#include "Event.hpp"

class EventManager
{
  private:
	std::vector<Event> _events;

  public:
	EventManager();
	EventManager(const EventManager &src);
	EventManager &operator=(const EventManager &src);
	EventManager(EventManager &&src) noexcept;
	EventManager &operator=(EventManager &&src) noexcept;
	~EventManager();

	void addEvent(const Event &event);
	const std::vector<Event> &getEvents() const;
};

#endif
