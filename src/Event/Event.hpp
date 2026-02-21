/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>

class Event
{
  private:
	std::string _name;
	double _probability;
	std::string _duration;
	std::string _node;

  public:
	Event(const std::string &name, double probability,
		  const std::string &duration, const std::string &node);
	Event(const Event &src);
	Event &operator=(const Event &src);
	~Event();

	const std::string &getName() const;
	double getProbability() const;
	const std::string &getDuration() const;
	const std::string &getNode() const;
};

#endif
