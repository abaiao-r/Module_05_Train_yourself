/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 18:26:44 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <set>
#include <utility>
#include "InputHandler.hpp"

#include <cctype>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <sstream>

#include "TrainFactory.hpp"

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

static void requireEndOfLine(std::istringstream &iss)
{
	iss >> std::ws;
	if (!iss.eof())
		throw InputHandler::ParseException("Unexpected trailing content");
}

static double parseDuration(const std::string &str)
{
	if (str.empty())
		throw InputHandler::ParseException("Empty duration string");
	char unit = str.back();
	double value;
	try
	{
		size_t used = 0;
		std::string num = str.substr(0, str.size() - 1);
		value = std::stod(num, &used);
		if (used != num.size())
			throw std::invalid_argument(num);
	}
	catch (const std::exception &)
	{
		throw InputHandler::ParseException(
			"Invalid duration value: " + str);
	}
	if (!std::isfinite(value) || value < 0.0)
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
		size_t used = 0;
		std::string h = str.substr(0, hPos);
		hours = std::stoi(h, &used);
		if (used != h.size())
			throw std::invalid_argument(h);
		if (hPos + 1 < str.size())
		{
			std::string m = str.substr(hPos + 1);
			minutes = std::stoi(m, &used);
			if (used != m.size())
				throw std::invalid_argument(m);
		}
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

SimulationData InputHandler::loadData(const std::string &networkFile,
									  const std::string &trainFile)
{
	SimulationData data;
	parseNetworkFile(networkFile, data.network, data.events);
	data.trains = parseTrainFile(trainFile);
	validateData(networkFile, trainFile, data);
	return data;
}

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
				requireEndOfLine(iss);
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
				requireEndOfLine(iss);
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
				requireEndOfLine(iss);
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
	std::set<std::pair<std::string, double>> seen;
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
			requireEndOfLine(iss);
			double depTime = parseTime(timeStr);
			trains.push_back(TrainFactory::createTrain(
				name, weight, friction, accel, brake, departure,
				arrival, depTime, parseTime(stopStr)));
			if (!seen.insert({name, depTime}).second)
				throw ParseException("Duplicate train '" + name
					+ "' with departure time " + timeStr);
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

void InputHandler::validateData(const std::string &networkFile,
								const std::string &trainFile,
								const SimulationData &data)
{
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
			bool linked = false;
			for (const auto &e : data.network.getNeighbours(
					 ev.getNodeName()))
				if (e.destination->getName() == ev.getNodeName2())
					linked = true;
			for (const auto &e : data.network.getNeighbours(
					 ev.getNodeName2()))
				if (e.destination->getName() == ev.getNodeName())
					linked = true;
			if (!linked)
				throw ParseException(
					networkFile + ": Event '" + ev.getName()
					+ "' references nonexistent rail '"
					+ ev.getNodeName() + " - " + ev.getNodeName2()
					+ "'");
		}
	}

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
