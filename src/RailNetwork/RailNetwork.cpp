/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RailNetwork.hpp"

#include <iostream>

#include "colours.hpp"

/* ---- Node management ---- */
void RailNetwork::addNode(std::shared_ptr<Node> node)
{
	if (_adjacencyList.find(node) == _adjacencyList.end())
		_adjacencyList[node] = std::vector<Edge>();
}

/* ---- Connection management ---- */
void RailNetwork::addConnection(std::shared_ptr<Node> node1,
								std::shared_ptr<Node> node2, double distance,
								double speedLimit)
{
	if (node1 == node2)
		throw Node::SelfEdgeException(node1->getName());

	addNode(node1);
	addNode(node2);

	for (const auto &edge : _adjacencyList[node1])
	{
		auto neighbor = edge.node.lock();
		if (neighbor && neighbor == node2)
			throw ConnectionAlreadyExistsException(node1->getName(),
												   node2->getName());
	}

	_adjacencyList[node1].push_back({node2, distance, speedLimit});
	_adjacencyList[node2].push_back({node1, distance, speedLimit});
	node1->addEdge(node2, distance, speedLimit);
	node2->addEdge(node1, distance, speedLimit);
}

/* ---- Queries ---- */
const std::vector<Edge> &RailNetwork::getNeighbours(
	std::shared_ptr<Node> node) const
{
	auto it = _adjacencyList.find(node);
	if (it != _adjacencyList.end())
		return it->second;
	throw std::runtime_error("Node does not exist in the network.");
}

std::vector<std::shared_ptr<Node>> RailNetwork::getNodes() const
{
	std::vector<std::shared_ptr<Node>> nodes;
	nodes.reserve(_adjacencyList.size());
	for (const auto &pair : _adjacencyList)
		nodes.push_back(pair.first);
	return nodes;
}

/* ---- Debug ---- */
void RailNetwork::printNetwork() const
{
	std::cout << CYAN
			  << "==================================================\n"
			  << "               RAIL NETWORK OVERVIEW\n"
			  << "==================================================" << RESET
			  << "\n";

	for (const auto &pair : _adjacencyList)
	{
		std::cout << BLUE << "[Node] " << pair.first->getName() << RESET
				  << "\n";
		std::cout << YELLOW << "  Neighbors:" << RESET << "\n";
		for (const auto &edge : pair.second)
		{
			auto neighbor = edge.node.lock();
			if (neighbor)
			{
				std::cout << "    " << GREEN << neighbor->getName() << RESET
						  << " [distance: " << edge.distance
						  << ", speed limit: " << edge.speedLimit << "]\n";
			}
		}
		std::cout << "\n";
	}
}

/* ---- Exception implementation ---- */
RailNetwork::ConnectionAlreadyExistsException::
	ConnectionAlreadyExistsException(const std::string &n1,
									 const std::string &n2)
	: std::runtime_error("Connection already exists: " + n1 + " <-> " + n2)
{
}
