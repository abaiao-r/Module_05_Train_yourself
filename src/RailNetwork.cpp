/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 14:22:37 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 14:47:26 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/RailNetwork.hpp"

#include <iostream>

#include "../includes/colours.hpp"

RailNetwork::RailNetwork()
{
}

RailNetwork::~RailNetwork()
{
}

void RailNetwork::addNode(const std::shared_ptr<Node> &node)
{
	_nodes[node->getName()] = node;
}

std::shared_ptr<Node> RailNetwork::getNode(const std::string &name) const
{
	auto it = _nodes.find(name);
	if (it != _nodes.end())
		return it->second;
	return nullptr;
}

const std::unordered_map<std::string, std::shared_ptr<Node>>
	&RailNetwork::getNodes() const
{
	return _nodes;
}

std::vector<Edge> RailNetwork::getAllEdges() const
{
	std::vector<Edge> allEdges;
	for (const auto &pair : _nodes)
	{
		const auto &node = pair.second;
		for (const auto &edge : node->getEdges())
		{
			allEdges.push_back(edge);
		}
	}
	return allEdges;
}

void RailNetwork::printNetwork() const
{
	std::cout << CYAN << "==================================================" << std::endl;
	std::cout << "               RAIL NETWORK OVERVIEW              " << std::endl;
	std::cout << "==================================================" << RESET << std::endl;

	for (const auto &pair : _nodes)
	{
		const auto &node = pair.second;
		std::cout << BLUE << "[Node] " << node->getName() << RESET << std::endl;
		std::cout << YELLOW << "  Edges:" << RESET << std::endl;
		for (const auto &edge : node->getEdges())
		{
			std::cout << "    " << GREEN << edge.getNode2() << RESET
					  << " [distance: " << edge.getDistance()
					  << ", speed limit: " << edge.getSpeedLimit() << "]"
					  << std::endl;
			// Print edge events
			for (const auto &event : edge.getEvents())
			{
				std::cout << "      " << MAGENTA << "Event: " << event.getEventName()
						  << " (prob: " << event.getEventProbability()
						  << ", duration: " << event.getEventDuration().count()
						  << "s, loc: " << event.getEventLocalization() << ")"
						  << RESET << std::endl;
			}
		}
		// Print node events
		if (!node->getEvents().empty())
		{
			std::cout << YELLOW << "  Node Events:" << RESET << std::endl;
			for (const auto &event : node->getEvents())
			{
				std::cout << "    " << MAGENTA << "Event: " << event.getEventName()
						  << " (prob: " << event.getEventProbability()
						  << ", duration: " << event.getEventDuration().count()
						  << "s, loc: " << event.getEventLocalization() << ")"
						  << RESET << std::endl;
			}
		}
		std::cout << std::endl;
	}
	std::cout << CYAN << "==================================================" << RESET << std::endl;
}
