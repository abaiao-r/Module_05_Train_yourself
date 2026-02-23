/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileOutputObserver.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileOutputObserver.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

/* ---- Static helpers ---- */
std::string FileOutputObserver::fmtTimeShort(double seconds)
{
	int total = static_cast<int>(seconds);
	int h = total / 3600;
	int m = (total % 3600) / 60;
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << h << "h"
		<< std::setw(2) << m;
	return oss.str();
}

static void ensureDir(const std::string &path)
{
	mkdir(path.c_str(), 0755);
}

std::string FileOutputObserver::buildFilename(const std::string &trainName,
											  double departureTimeSec)
{
	ensureDir("output");
	ensureDir("output/results");
	return "output/results/" + trainName + "_" + fmtTimeShort(departureTimeSec)
		   + ".result";
}

std::string FileOutputObserver::padRight(const std::string &s,
										 size_t width)
{
	if (s.size() >= width)
		return s;
	return std::string(width - s.size(), ' ') + s;
}

/* ---- Constructor / Destructor ---- */
FileOutputObserver::FileOutputObserver(const std::string &trainName,
									   double departureTimeSec)
	: _filename(buildFilename(trainName, departureTimeSec))
{
	_file.open(_filename);
	if (!_file.is_open())
		throw std::runtime_error("Cannot create output file: " + _filename);
}

FileOutputObserver::~FileOutputObserver()
{
	if (_file.is_open())
		_file.close();
}

/* ---- Observer callbacks ---- */
void FileOutputObserver::onTrainStart(const std::string &trainName,
									  int trainId,
									  double estimatedTimeSec)
{
	_file << "Train : " << trainName << trainId << "\n";
	_file << "Final travel time : " << fmtTimeShort(estimatedTimeSec)
		  << "m\n\n";
}

void FileOutputObserver::onTrainStep(double timeSinceStart,
									 const std::string &fromNode,
									 const std::string &toNode,
									 double distRemainingKm,
									 const std::string &action,
									 int positionCellKm,
									 int segmentCellsKm,
									 const std::vector<int> &blockingCells)
{
	// Time
	_file << "[" << fmtTimeShort(timeSinceStart) << "] - ";

	// Segment nodes (right-padded to 10 chars)
	_file << "[" << padRight(fromNode, 10) << "]"
		  << "[" << padRight(toNode, 10) << "] - ";

	// Distance remaining
	std::ostringstream distOss;
	distOss << std::fixed << std::setprecision(2)
			<< std::setfill('0') << std::setw(5) << distRemainingKm;
	_file << "[" << distOss.str() << "km] - ";

	// Action (right-padded to 8 chars)
	_file << "[" << padRight(action, 8) << "] - ";

	// Rail graph: one cell per km
	for (int km = 0; km < segmentCellsKm; km++)
	{
		bool isBlocking = false;
		for (int bp : blockingCells)
		{
			if (bp == km)
			{
				isBlocking = true;
				break;
			}
		}
		if (km == positionCellKm)
			_file << "[x]";
		else if (isBlocking)
			_file << "[O]";
		else
			_file << "[ ]";
	}
	_file << "\n";
}

void FileOutputObserver::onTrainEvent(const std::string &eventName,
									  const std::string &nodeName,
									  double delaySec)
{
	_file << "  ** EVENT: \"" << eventName << "\" at " << nodeName
		  << " (+";
	if (delaySec >= 3600.0)
		_file << fmtTimeShort(delaySec);
	else
		_file << static_cast<int>(delaySec / 60) << "min";
	_file << " delay)\n";
}

void FileOutputObserver::onTrainFinish(const std::string &trainName,
									   double totalTimeSec)
{
	(void)trainName;
	_file << "\nActual travel time : " << fmtTimeShort(totalTimeSec)
		  << "m\n";
	_file.flush();
}

const std::string &FileOutputObserver::getFilename() const
{
	return _filename;
}
