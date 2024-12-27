/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventManager.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 13:51:26 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/20 12:25:48 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTMANAGER_HPP
#define EVENTMANAGER_HPP

#include "Event.hpp"
#include "libraries.hpp"

class EventManager
{
	private:
		std::vector<Event> _events;

	public:
		EventManager();
		EventManager(const EventManager &src);
		EventManager &operator=(const EventManager &src);
		EventManager(EventManager &&src);
		EventManager &operator=(EventManager &&src);
		~EventManager();

		// Getters
		const std::vector<Event> &getEvents() const;
};

#endif
