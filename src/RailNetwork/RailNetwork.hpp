/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r           #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r          ###   ########.fr       */
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

class RailNetwork
{
  private:
	std::unordered_map<std::string, std::shared_ptr<Node>> _nodes;
	std::unordered_map<std::string, std::vector<Edge>> _adjacencyList;

  public:
	RailNetwork();
	RailNetwork(const RailNetwork &src);
	RailNetwork &operator=(const RailNetwork &src);
	RailNetwork(RailNetwork &&src) noexcept;
	RailNetwork &operator=(RailNetwork &&src) noexcept;
	~RailNetwork();

	void addNode(const std::string &name);
	void addConnection(const std::string &from, const std::string &to,
					   double distance, double speedLimit);
	std::shared_ptr<Node> findNode(const std::string &name) const;
	const std::vector<Edge> &getNeighbours(
		const std::string &nodeName) const;
	std::vector<std::string> getNodeNames() const;
	size_t nodeCount() const;

	class NodeNotFoundException : public std::runtime_error
	{
	  public:
		explicit NodeNotFoundException(const std::string &name);
	};

	class DuplicateNodeException : public std::runtime_error
	{
	  public:
		explicit DuplicateNodeException(const std::string &name);
	};

	class DuplicateConnectionException : public std::runtime_error
	{
	  public:
		DuplicateConnectionException(const std::string &from,
									 const std::string &to);
	};
};

#endif
