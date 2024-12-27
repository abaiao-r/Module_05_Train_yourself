/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 21:54:26 by andrefranci       #+#    #+#             */
/*   Updated: 2024/10/17 15:34:29 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RAILNETWORK_HPP
#define RAILNETWORK_HPP

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Node.hpp"
#include "Singleton.hpp"
#include "libraries.hpp"

class RailNetwork : public Singleton<RailNetwork>
{
	private:
		// Adjacency list representation of the rail network
		// The key is a shared pointer to a node and the value is a vector of
		// pairs The pair contains a shared pointer to a node and the distance
		// between the two nodes
		using AdjacencyList = std::unordered_map<
			std::shared_ptr<Node>,
			std::vector<std::pair<std::shared_ptr<Node>, size_t>>>;
		AdjacencyList _adjacencyList;

	public:
		// Add a node to the rail network
		void addNode(std::shared_ptr<Node> node);

		// Add a connection between two nodes in the rail network
		void addConnection(std::shared_ptr<Node> node1,
						   std::shared_ptr<Node> node2, size_t distance);

		// Get the neighbors of a node
		const std::vector<std::pair<std::shared_ptr<Node>, size_t>>
			&getNeighbours(std::shared_ptr<Node> node) const;

		// Get all nodes in the rail network
		const std::vector<std::shared_ptr<Node>> getNodes() const;
};

#endif
