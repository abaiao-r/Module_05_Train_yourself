/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 21:54:26 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/18 00:38:30 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RAILNETWORK_HPP
#define RAILNETWORK_HPP

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Edge.hpp"
#include "Node.hpp"
#include "Singleton.hpp"
#include "libraries.hpp"

class RailNetwork : public Singleton<RailNetwork>
{
	private:
		// Use Node::Edge for adjacency list
		using AdjacencyList
			= std::unordered_map<std::shared_ptr<Node>, std::vector<Edge>>;
		AdjacencyList _adjacencyList;

	public:
		// Add a node to the rail network
		void addNode(std::shared_ptr<Node> node);

		// Add a connection between two nodes in the rail network
		void addConnection(std::shared_ptr<Node> node1,
						   std::shared_ptr<Node> node2, size_t distance,
						   size_t speedLimit);

		// Get the neighbors of a node
		const std::vector<Edge> &getNeighbours(
			std::shared_ptr<Node> node) const;

		// Get all nodes in the rail network
		const std::vector<std::shared_ptr<Node>> getNodes() const;

		// debug function
		void printNetwork() const;

		// Custom exception class for existing connection
		class ConnectionAlreadyExistsException : public std::runtime_error
		{
			public:
				ConnectionAlreadyExistsException(const std::string &node1Name,
												 const std::string &node2Name)
					: std::runtime_error(
						"Error: Connection already exists between " + node1Name
						+ " and " + node2Name)
				{
				}
		};
};

#endif
