/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/22 13:05:50 by ctw03933         ###   ########.fr       */
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
	double value;
	try
	{
		value = std::stod(str.substr(0, str.size() - 1));
	}
	catch (const std::exception &)
	{
		throw InputHandler::ParseException(
			"Invalid duration value: " + str);
	}
	if (value < 0.0)
		throw InputHandler::ParseException(
			"Duration cannot be negative: " + str);
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
	int hours, minutes = 0;
	try
	{
		hours = std::stoi(str.substr(0, hPos));
		if (hPos + 1 < str.size())
			minutes = std::stoi(str.substr(hPos + 1));
	}
	catch (const std::exception &)
	{
		throw InputHandler::ParseException(
			"Invalid time value: " + str);
	}
	if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59)
		throw InputHandler::ParseException(
			"Time out of range: " + str);
	return hours * 3600.0 + minutes * 60.0;
}

/* ---- Public interface ---- */
SimulationData InputHandler::loadData(const std::string &networkFile,
									  const std::string &trainFile)
{
	SimulationData data;
	parseNetworkFile(networkFile, data.network, data.events);
	data.trains = parseTrainFile(trainFile);
	validateData(networkFile, trainFile, data);
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
	int lineNum = 0;
	while (std::getline(file, line))
	{
		++lineNum;
		if (line.empty())
			continue;

		std::istringstream iss(line);
		std::string keyword;
		iss >> keyword;

		try
		{
			if (keyword == "Node")
			{
				std::string name = parseQuotedOrWord(iss);
				if (name.empty())
					throw ParseException("Missing node name");
				network.addNode(name);
			}
			else if (keyword == "Event")
			{
				std::string name = parseQuotedOrWord(iss);
				double probability;
				std::string durationStr, nodeName, nodeName2;
				iss >> probability >> durationStr >> nodeName;
				if (name.empty() || durationStr.empty()
					|| nodeName.empty())
					throw ParseException("Invalid event line");
				/* Optional second node → rail segment event */
				iss >> nodeName2;
				events.emplace_back(name, probability,
									parseDuration(durationStr), nodeName,
									nodeName2);
			}
			else if (keyword == "Rail")
			{
				std::string from, to;
				double distance, speedLimit;
				iss >> from >> to >> distance >> speedLimit;
				if (iss.fail() || from.empty() || to.empty())
					throw ParseException("Invalid rail line");
				network.addConnection(from, to, distance, speedLimit);
			}
			else
			{
				throw ParseException("Unknown keyword '" + keyword
									 + "'");
			}
		}
		catch (const std::exception &e)
		{
			throw ParseException(filepath + ":" + std::to_string(lineNum)
								 + ": " + e.what());
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
	int lineNum = 0;
	while (std::getline(file, line))
	{
		++lineNum;
		if (line.empty())
			continue;

		try
		{
			std::istringstream iss(line);
			std::string name;
			double weight, friction, accel, brake;
			std::string departure, arrival, timeStr, stopStr;
			iss >> name >> weight >> friction >> accel >> brake
				>> departure >> arrival >> timeStr >> stopStr;
			if (iss.fail() || name.empty() || departure.empty()
				|| arrival.empty() || timeStr.empty()
				|| stopStr.empty())
				throw ParseException("Invalid train line");
			trains.push_back(TrainFactory::createTrain(
				name, weight, friction, accel, brake, departure,
				arrival, parseTime(timeStr), parseTime(stopStr)));
		}
		catch (const std::exception &e)
		{
			throw ParseException(filepath + ":"
								 + std::to_string(lineNum) + ": "
								 + e.what());
		}
	}
	return trains;
}

InputHandler::ParseException::ParseException(const std::string &msg)
	: std::runtime_error(msg)
{
}

/* ---- Cross-file validation ---- */
void InputHandler::validateData(const std::string &networkFile,
								const std::string &trainFile,
								const SimulationData &data)
{
	/* Network must contain at least one node and one rail */
	if (data.network.nodeCount() == 0)
		throw ParseException(networkFile
							 + ": Network file contains no nodes");
	auto names = data.network.getNodeNames();
	bool hasRails = false;
	for (const auto &n : names)
	{
		if (!data.network.getNeighbours(n).empty())
		{
			hasRails = true;
			break;
		}
	}
	if (!hasRails)
		throw ParseException(networkFile
							 + ": Network file contains no rails");

	/* Every event must reference existing node(s) */
	for (const auto &ev : data.events)
	{
		bool found = false;
		for (const auto &n : names)
		{
			if (n == ev.getNodeName())
			{
				found = true;
				break;
			}
		}
		if (!found)
			throw ParseException(
				networkFile + ": Event '" + ev.getName()
				+ "' references unknown node '" + ev.getNodeName()
				+ "'");
		/* Rail segment event: validate second node too */
		if (ev.isRailEvent())
		{
			bool found2 = false;
			for (const auto &n : names)
			{
				if (n == ev.getNodeName2())
				{
					found2 = true;
					break;
				}
			}
			if (!found2)
				throw ParseException(
					networkFile + ": Event '" + ev.getName()
					+ "' references unknown node '"
					+ ev.getNodeName2() + "'");
		}
	}

	/* Every train must depart from and arrive at an existing node */
	for (const auto &train : data.trains)
	{
		bool depFound = false, arrFound = false;
		for (const auto &n : names)
		{
			if (n == train->getDepartureStation())
				depFound = true;
			if (n == train->getArrivalStation())
				arrFound = true;
		}
		if (!depFound)
			throw ParseException(
				trainFile + ": Train '" + train->getName()
				+ "' departs from unknown station '"
				+ train->getDepartureStation() + "'");
		if (!arrFound)
			throw ParseException(
				trainFile + ": Train '" + train->getName()
				+ "' arrives at unknown station '"
				+ train->getArrivalStation() + "'");
	}
}
