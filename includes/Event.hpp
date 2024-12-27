/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 12:14:08 by andrefranci       #+#    #+#             */
/*   Updated: 2024/09/20 12:17:41 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENT_HPP
#define EVENT_HPP

#include "libraries.hpp"

class Event
{
	private:
		std::string _eventName;
		float		_eventProbability;
		// std::chrono::duration<int> _eventDuration;
		std::string _node;	// node where the event will happen

	public:
		Event();
		Event(const Event &src);
		~Event();
		Event &operator=(const Event &src);

		// Getters
		const std::string &getEventName() const;
		const float		&getEventProbability() const;
		// const std::chrono::duration<int> &getEventDuration() const;
		const std::string &getNode() const;
};

#endif
