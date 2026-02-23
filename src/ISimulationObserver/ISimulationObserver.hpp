/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ISimulationObserver.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ISIMULATIONOBSERVER_HPP
#define ISIMULATIONOBSERVER_HPP

#include <string>
#include <vector>

/**
 * Observer interface for simulation events (Observer Design Pattern).
 * Concrete observers receive real-time notifications as trains progress
 * through the network, enabling decoupled output strategies (file, console,
 * logging, etc.).
 */
class ISimulationObserver
{
  public:
	virtual ~ISimulationObserver() = default;

	/** Called once before the simulation loop starts for a train. */
	virtual void onTrainStart(const std::string &trainName, int trainId,
							  double estimatedTimeSec) = 0;

	/** Called at each output interval during the simulation. */
	virtual void onTrainStep(double timeSinceStart,
							 const std::string &fromNode,
							 const std::string &toNode,
							 double distRemainingKm,
							 const std::string &action,
							 int positionCellKm,
							 int segmentCellsKm,
							 const std::vector<int> &blockingCells) = 0;

	/** Called when a random event triggers on a train. */
	virtual void onTrainEvent(const std::string &eventName,
							  const std::string &nodeName,
							  double delaySec) = 0;

	/** Called once when the train finishes its journey. */
	virtual void onTrainFinish(const std::string &trainName,
							   double totalTimeSec) = 0;
};

#endif
