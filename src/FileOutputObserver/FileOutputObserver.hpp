/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileOutputObserver.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEOUTPUTOBSERVER_HPP
#define FILEOUTPUTOBSERVER_HPP

#include <fstream>
#include <string>

#include "ISimulationObserver.hpp"

/**
 * Concrete Observer that writes per-train result files.
 * Output filename: TrainName_DepartureTime.result
 */
class FileOutputObserver : public ISimulationObserver
{
  private:
	std::ofstream _file;
	std::string _filename;

	static std::string buildFilename(const std::string &trainName,
									 double departureTimeSec);
	static std::string fmtTimeShort(double seconds);
	static std::string padRight(const std::string &s, size_t width);

  public:
	FileOutputObserver(const std::string &trainName,
					   double departureTimeSec);
	FileOutputObserver(const FileOutputObserver &) = delete;
	FileOutputObserver &operator=(const FileOutputObserver &) = delete;
	~FileOutputObserver();

	void onTrainStart(const std::string &trainName, int trainId,
					  double estimatedTimeSec) override;
	void onTrainStep(double timeSinceStart,
					 const std::string &fromNode,
					 const std::string &toNode,
					 double distRemainingKm,
					 const std::string &action,
					 int positionCellKm,
					 int segmentCellsKm,
					 const std::vector<int> &blockingCells) override;
	void onTrainEvent(const std::string &eventName,
					  const std::string &nodeName,
					  double delaySec) override;
	void onTrainFinish(const std::string &trainName,
					   double totalTimeSec) override;

	const std::string &getFilename() const;
};

#endif
