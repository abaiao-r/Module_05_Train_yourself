/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetworkTest.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 14:22:37 by andrefranci       #+#    #+#             */
/*   Updated: 2025/03/08 11:51:51 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Edge.hpp"
#include "../includes/RailNetwork.hpp"

// Add a node to the rail network
void RailNetwork::addNode(std::shared_ptr<Node> node)
{
	if (_adjacencyList.find(node) == _adjacencyList.end())
	{
		_adjacencyList[node] = std::vector<Edge>();
	}
}

// Add a connection (edge) between two nodes with a specific distance and speed limit
void RailNetwork::addConnection(std::shared_ptr<Node> node1,
								std::shared_ptr<Node> node2,
								size_t distance,
								size_t speedLimit)
{
	if (node1 == node2)
	{
		throw Node::SelfEdgeException(node1->getName());
	}

	// Ensure both nodes exist in the network, otherwise add them
	addNode(node1);
	addNode(node2);

	// Check if the connection already exists
	for (const auto &edge : _adjacencyList[node1])
	{
		auto neighbor = edge.node.lock();
		if (neighbor && neighbor == node2)
		{
			throw ConnectionAlreadyExistsException(node1->getName(),
												   node2->getName());
		}
	}

	// Add the connection in both directions (undirected graph)
	_adjacencyList[node1].push_back({node2, distance, speedLimit});
	_adjacencyList[node2].push_back({node1, distance, speedLimit});

	// Keep Node's edge list in sync
	node1->addEdge(node2, distance, speedLimit);
	node2->addEdge(node1, distance, speedLimit);
}

// Get the neighbors of a node
const std::vector<Edge>
	&RailNetwork::getNeighbours(std::shared_ptr<Node> node) const
{
	auto it = _adjacencyList.find(node);
	if (it != _adjacencyList.end())
	{
		return it->second;
	}
	throw std::runtime_error("Node does not exist in the network");
}

// Get all nodes in the rail network
const std::vector<std::shared_ptr<Node>> RailNetwork::getNodes() const
{
	std::vector<std::shared_ptr<Node>> nodes;
	for (const auto &pair : _adjacencyList)
	{
		nodes.push_back(pair.first);
	}
	return nodes;
}

// Debug function to print the rail network
void RailNetwork::printNetwork() const
{
	std::cout << CYAN << "=================================================="
			  << std::endl;
	std::cout << "               RAIL NETWORK OVERVIEW              "
			  << std::endl;
	std::cout << "==================================================" << RESET
			  << std::endl;

	for (const auto &pair : _adjacencyList)
	{
		std::cout << BLUE << "[Node] " << pair.first->getName() << RESET
				  << std::endl;
		std::cout << YELLOW << "  Neighbors:" << RESET << std::endl;
		for (const auto &edge : pair.second)
		{
			auto neighbor = edge.node.lock();
			if (neighbor)
			{
				std::cout << "    " << GREEN << neighbor->getName() << RESET
						  << " [distance: " << edge.distance
						  << ", speed limit: " << edge.speedLimit << "]"
						  << std::endl;
			}
		}
		std::cout << std::endl;
	}
}