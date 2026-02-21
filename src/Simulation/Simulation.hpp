/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simulation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <memory>
#include <random>
#include <vector>

#include "Event.hpp"
#include "IPathfinding.hpp"
#include "OutputManager.hpp"
#include "RailNetwork.hpp"
#include "Train.hpp"

class Simulation
{
  private:
	RailNetwork _network;
	std::vector<std::unique_ptr<Train>> _trains;
	std::vector<Event> _events;
	std::unique_ptr<IPathfinding> _pathfinder;
	OutputManager _output;
	std::mt19937 _rng;

  public:
	Simulation(RailNetwork network,
			   std::vector<std::unique_ptr<Train>> trains,
			   std::vector<Event> events,
			   std::unique_ptr<IPathfinding> pathfinder);
	Simulation(const Simulation &) = delete;
	Simulation &operator=(const Simulation &) = delete;
	~Simulation();

	void run();

  private:
	void computePaths();
	void simulateTrain(Train &train);
	double getEdgeTravelTime(const std::string &from,
							 const std::string &to) const;
	std::vector<const Event *> getEventsAtNode(
		const std::string &nodeName) const;
};

#endif
