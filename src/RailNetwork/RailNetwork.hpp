/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RAILNETWORK_HPP
#define RAILNETWORK_HPP

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "Edge.hpp"
#include "Node.hpp"
#include "Singleton.hpp"

class RailNetwork : public Singleton<RailNetwork>
{
  private:
	using AdjacencyList =
		std::unordered_map<std::shared_ptr<Node>, std::vector<Edge>>;
	AdjacencyList _adjacencyList;

  public:
	void addNode(std::shared_ptr<Node> node);
	void addConnection(std::shared_ptr<Node> node1,
					   std::shared_ptr<Node> node2, double distance,
					   double speedLimit);
	const std::vector<Edge> &getNeighbours(
		std::shared_ptr<Node> node) const;
	std::vector<std::shared_ptr<Node>> getNodes() const;
	void printNetwork() const;

	class ConnectionAlreadyExistsException : public std::runtime_error
	{
	  public:
		ConnectionAlreadyExistsException(const std::string &n1,
										 const std::string &n2);
	};
};

#endif
