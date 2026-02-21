/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r           #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENT_HPP
#define EVENT_HPP

#include <random>
#include <string>

class Event
{
  private:
	std::string _name;
	double _probability;
	double _duration;  // seconds
	std::string _nodeName;

  public:
	Event(const std::string &name, double probability, double duration,
		  const std::string &nodeName);
	Event(const Event &src);
	Event &operator=(const Event &src);
	~Event();

	bool tryTrigger(std::mt19937 &rng) const;
	const std::string &getName() const;
	double getProbability() const;
	double getDuration() const;
	const std::string &getNodeName() const;
};

#endif
