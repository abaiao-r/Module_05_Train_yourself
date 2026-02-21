/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "InputHandler.hpp"

#include <fstream>
#include <sstream>
#include <utility>

/* ---- Canonical form ---- */
InputHandler::InputHandler(const std::string &railNetworkFile,
						   const std::string &trainFile)
	: _railNetworkFilePath(railNetworkFile), _trainFilePath(trainFile)
{
}

InputHandler::InputHandler(const InputHandler &src) { *this = src; }

InputHandler &InputHandler::operator=(const InputHandler &src)
{
	if (this != &src)
	{
		_railNetworkFilePath = src._railNetworkFilePath;
		_trainFilePath = src._trainFilePath;
	}
	return *this;
}

InputHandler::InputHandler(InputHandler &&src) noexcept
{
	*this = std::move(src);
}

InputHandler &InputHandler::operator=(InputHandler &&src) noexcept
{
	if (this != &src)
	{
		_railNetworkFilePath = std::move(src._railNetworkFilePath);
		_trainFilePath = std::move(src._trainFilePath);
	}
	return *this;
}

InputHandler::~InputHandler() {}

/* ---- Parsing ---- */
void InputHandler::loadRailNetworkData()
{
	std::ifstream file(_railNetworkFilePath);
	if (!file.is_open())
		throw InputException("Failed to open file: " + _railNetworkFilePath);
	if (file.peek() == std::ifstream::traits_type::eof())
		throw InputException("File is empty: " + _railNetworkFilePath);

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
			std::string nodeName;
			iss >> nodeName;
			if (nodeName.empty())
				throw InputException("Missing node name in: " + line);
			// TODO: create node via RailNetwork
		}
		else if (keyword == "Event")
		{
			std::string eventName;
			double probability;
			std::string duration;
			std::string nodeName;
			iss >> eventName >> probability >> duration >> nodeName;
			if (eventName.empty() || nodeName.empty())
				throw InputException("Invalid event line: " + line);
			// TODO: create event via EventManager
		}
		else if (keyword == "Rail")
		{
			std::string startNode, endNode;
			double distance, speedLimit;
			iss >> startNode >> endNode >> distance >> speedLimit;
			if (startNode.empty() || endNode.empty())
				throw InputException("Invalid rail line: " + line);
			// TODO: create connection via RailNetwork
		}
		else
		{
			throw InputException("Unknown keyword '" + keyword + "' in: "
								 + line);
		}
	}
}

void InputHandler::loadTrainData()
{
	std::ifstream file(_trainFilePath);
	if (!file.is_open())
		throw InputException("Failed to open file: " + _trainFilePath);
	if (file.peek() == std::ifstream::traits_type::eof())
		throw InputException("File is empty: " + _trainFilePath);

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty())
			continue;

		std::istringstream iss(line);
		std::string name;
		double acceleration, braking;
		std::string departure, arrival, time;
		iss >> name >> acceleration >> braking >> departure >> arrival >> time;
		if (name.empty() || departure.empty() || arrival.empty())
			throw InputException("Invalid train line: " + line);
		// TODO: create train via TrainFactory
	}
}

/* ---- Exception implementation ---- */
InputHandler::InputException::InputException(const std::string &msg)
	: std::runtime_error(msg)
{
}
