/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r           #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "InputHandler.hpp"

#include <cctype>
#include <fstream>
#include <sstream>

#include "TrainFactory.hpp"

/* ---- File-scope helpers ---- */
static std::string parseQuotedOrWord(std::istringstream &iss)
{
	char c;
	while (iss.get(c) && std::isspace(static_cast<unsigned char>(c)))
		;
	if (!iss)
		return "";
	if (c == '"')
	{
		std::string result;
		std::getline(iss, result, '"');
		return result;
	}
	std::string word;
	word += c;
	while (iss.get(c) && !std::isspace(static_cast<unsigned char>(c)))
		word += c;
	return word;
}

static double parseDuration(const std::string &str)
{
	if (str.empty())
		throw InputHandler::ParseException("Empty duration string");
	char unit = str.back();
	double value = std::stod(str.substr(0, str.size() - 1));
	switch (unit)
	{
		case 'm':
			return value * 60.0;
		case 'h':
			return value * 3600.0;
		case 'd':
			return value * 86400.0;
		default:
			throw InputHandler::ParseException(
				"Unknown duration unit: " + std::string(1, unit));
	}
}

static double parseTime(const std::string &str)
{
	size_t hPos = str.find('h');
	if (hPos == std::string::npos)
		throw InputHandler::ParseException("Invalid time format: " + str);
	int hours = std::stoi(str.substr(0, hPos));
	int minutes = 0;
	if (hPos + 1 < str.size())
		minutes = std::stoi(str.substr(hPos + 1));
	return hours * 3600.0 + minutes * 60.0;
}

/* ---- Public interface ---- */
SimulationData InputHandler::loadData(const std::string &networkFile,
									  const std::string &trainFile)
{
	SimulationData data;
	parseNetworkFile(networkFile, data.network, data.events);
	data.trains = parseTrainFile(trainFile);
	return data;
}

/* ---- Private helpers ---- */
void InputHandler::parseNetworkFile(const std::string &filepath,
									RailNetwork &network,
									std::vector<Event> &events)
{
	std::ifstream file(filepath);
	if (!file.is_open())
		throw ParseException("Cannot open file: " + filepath);

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty())
			continue;

		std::istringstream iss(line);
		std::string keyword;
		iss >> keyword;

		if (keyword == "Node")
		{
			std::string name = parseQuotedOrWord(iss);
			if (name.empty())
				throw ParseException("Missing node name: " + line);
			network.addNode(name);
		}
		else if (keyword == "Event")
		{
			std::string name = parseQuotedOrWord(iss);
			double probability;
			std::string durationStr, nodeName;
			iss >> probability >> durationStr >> nodeName;
			if (name.empty() || durationStr.empty() || nodeName.empty())
				throw ParseException("Invalid event line: " + line);
			events.emplace_back(name, probability,
								parseDuration(durationStr), nodeName);
		}
		else if (keyword == "Rail")
		{
			std::string from, to;
			double distance, speedLimit;
			iss >> from >> to >> distance >> speedLimit;
			if (from.empty() || to.empty())
				throw ParseException("Invalid rail line: " + line);
			network.addConnection(from, to, distance, speedLimit);
		}
		else
		{
			throw ParseException("Unknown keyword: " + keyword);
		}
	}
}

std::vector<std::unique_ptr<Train>> InputHandler::parseTrainFile(
	const std::string &filepath)
{
	std::ifstream file(filepath);
	if (!file.is_open())
		throw ParseException("Cannot open file: " + filepath);

	std::vector<std::unique_ptr<Train>> trains;
	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty())
			continue;

		std::istringstream iss(line);
		std::string name;
		double accel, brake;
		std::string departure, arrival, timeStr;
		iss >> name >> accel >> brake >> departure >> arrival >> timeStr;
		if (name.empty() || departure.empty() || arrival.empty()
			|| timeStr.empty())
			throw ParseException("Invalid train line: " + line);
		trains.push_back(TrainFactory::createTrain(
			name, accel, brake, departure, arrival, parseTime(timeStr)));
	}
	return trains;
}

InputHandler::ParseException::ParseException(const std::string &msg)
	: std::runtime_error(msg)
{
}
