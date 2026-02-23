/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OutputManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:00:53 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "OutputManager.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "Node.hpp"

/* ---- Canonical form ---- */
OutputManager::OutputManager() {}
OutputManager::OutputManager(const OutputManager &) {}
OutputManager &OutputManager::operator=(const OutputManager &)
{
	return *this;
}
OutputManager::~OutputManager() {}

/* ---- Formatting ---- */
std::string OutputManager::formatTime(double seconds)
{
	int totalSec = static_cast<int>(seconds);
	int h = (totalSec / 3600) % 24;
	int m = (totalSec % 3600) / 60;
	int s = totalSec % 60;
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << h << ":"
		<< std::setw(2) << m << ":"
		<< std::setw(2) << s;
	return oss.str();
}

/* ---- Output methods ---- */
void OutputManager::printNetwork(const RailNetwork &network) const
{
	std::cout << "=== Rail Network ===" << std::endl;
	for (const auto &name : network.getNodeNames())
	{
		std::cout << "  " << name << " -> ";
		const auto &edges = network.getNeighbours(name);
		for (size_t i = 0; i < edges.size(); i++)
		{
			if (i > 0)
				std::cout << ", ";
			std::cout << edges[i].destination->getName() << " ("
					  << edges[i].distance << "km, " << edges[i].speedLimit
					  << "km/h)";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void OutputManager::printTrainPath(const Train &train) const
{
	std::cout << train.getName() << " path: ";
	const auto &path = train.getPath();
	for (size_t i = 0; i < path.size(); i++)
	{
		if (i > 0)
			std::cout << " -> ";
		std::cout << path[i]->getName();
	}
	std::cout << std::endl;
}

void OutputManager::printDeparture(const Train &train) const
{
	std::cout << "[" << formatTime(train.getDepartureTime()) << "] "
			  << train.getName() << " departs from "
			  << train.getDepartureStation() << std::endl;
}

void OutputManager::printArrival(const Train &train,
								 const std::string &nodeName,
								 double time) const
{
	(void)train;
	std::cout << "  [" << formatTime(time) << "] -> " << nodeName
			  << std::endl;
}

void OutputManager::printEvent(const Event &event,
							   const Train &train) const
{
	std::cout << "  ** EVENT: \"" << event.getName() << "\" at "
			  << event.getNodeName() << " delays " << train.getName()
			  << " by " << (event.getDuration() / 60.0) << " min"
			  << std::endl;
}

void OutputManager::printResult(
	const std::vector<std::unique_ptr<Train>> &trains) const
{
	std::cout << std::endl << "=== Results ===" << std::endl;
	for (const auto &train : trains)
	{
		std::cout << train->getName() << ": "
				  << Train::statusToString(train->getStatus())
				  << " | Arrival: "
				  << formatTime(train->getCurrentTime()) << " | Delay: "
				  << (train->getTotalDelay() / 60.0) << " min"
				  << std::endl;
	}
}
