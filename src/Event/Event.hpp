/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/22 13:05:50 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENT_HPP
#define EVENT_HPP

#include <random>
#include <stdexcept>
#include <string>

class Event
{
  private:
	std::string _name;
	double _probability;
	double _duration;  // seconds
	std::string _nodeName;
	std::string _nodeName2;  // non-empty → rail segment event

  public:
	Event(const std::string &name, double probability, double duration,
		  const std::string &nodeName,
		  const std::string &nodeName2 = "");
	Event(const Event &src);
	Event &operator=(const Event &src);
	~Event();

	bool tryTrigger(std::mt19937 &rng) const;
	const std::string &getName() const;
	double getProbability() const;
	double getDuration() const;
	const std::string &getNodeName() const;
	const std::string &getNodeName2() const;
	bool isRailEvent() const;
};

#endif
