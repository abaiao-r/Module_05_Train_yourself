/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 14:22:37 by andrefranci       #+#    #+#             */
/*   Updated: 2024/10/17 15:34:27 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/RailNetwork.hpp"

// Add a node to the rail network
void RailNetwork::addNode(std::shared_ptr<Node> node)
{
	// If node is not already in the network, add it with an empty neighbor list
	if (_adjacencyList.find(node) == _adjacencyList.end())
	{
		_adjacencyList[node]
			= std::vector<std::pair<std::shared_ptr<Node>, size_t>>();
	}
}

// Add a connection (edge) between two nodes with a specific distance
void RailNetwork::addConnection(std::shared_ptr<Node> node1,
								std::shared_ptr<Node> node2, size_t distance)
{
	// Ensure both nodes exist in the network, otherwise add them
	addNode(node1);
	addNode(node2);

	// Add the connection in both directions (undirected graph)
	_adjacencyList[node1].push_back(std::make_pair(node2, distance));
	_adjacencyList[node2].push_back(std::make_pair(node1, distance));
}

// Get the neighbors of a node
const std::vector<std::pair<std::shared_ptr<Node>, size_t>>
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