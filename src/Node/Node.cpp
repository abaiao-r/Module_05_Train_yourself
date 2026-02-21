/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Node.hpp"

#include <limits>

/* ---- Static member ---- */
std::unordered_set<std::string> &Node::_existingNames()
{
	static auto *names = new std::unordered_set<std::string>();
	return *names;
}

/* ---- Constructor / Destructor ---- */
Node::Node(const std::string &name) : _name(name)
{
	if (name.empty())
		throw InvalidNodeException(name);
	if (_existingNames().count(name))
		throw InvalidNodeException(name);
	_existingNames().insert(name);
}

Node::~Node() { _existingNames().erase(_name); }

/* ---- Edge management ---- */
static void validateEdge(const std::string &from, const std::string &to,
						 double distance, double speedLimit)
{
	if (distance <= 0.0)
		throw Node::InvalidEdgeException(from, to,
										 "Distance must be positive.");
	if (speedLimit <= 0.0)
		throw Node::InvalidEdgeException(from, to,
										 "Speed limit must be positive.");
}

void Node::addEdge(std::weak_ptr<Node> node, double distance, double speedLimit)
{
	auto sharedNode = node.lock();
	if (!sharedNode)
		throw InvalidEdgeException(_name, "expired",
								   "Target node no longer exists.");
	if (sharedNode.get() == this)
		throw SelfEdgeException(_name);

	for (const auto &edge : _edges)
	{
		auto existing = edge.node.lock();
		if (existing && existing == sharedNode)
			throw EdgeAlreadyExistsException(_name, sharedNode->getName());
	}

	validateEdge(_name, sharedNode->getName(), distance, speedLimit);
	_edges.push_back({node, distance, speedLimit});
}

/* ---- Getters ---- */
const std::string &Node::getName() const { return _name; }
const std::vector<Edge> &Node::getEdges() const { return _edges; }

/* ---- Exception implementations ---- */
Node::SelfEdgeException::SelfEdgeException(const std::string &nodeName)
	: std::runtime_error("Node '" + nodeName + "' cannot point to itself.")
{
}

Node::EdgeAlreadyExistsException::EdgeAlreadyExistsException(
	const std::string &from, const std::string &to)
	: std::runtime_error("Edge already exists: " + from + " -> " + to)
{
}

Node::InvalidNodeException::InvalidNodeException(const std::string &nodeName)
	: std::runtime_error(
		  nodeName.empty() ? "Node name cannot be empty."
						   : "Node already exists: " + nodeName)
{
}

Node::InvalidEdgeException::InvalidEdgeException(const std::string &from,
												  const std::string &to,
												  const std::string &reason)
	: std::runtime_error("Invalid edge " + from + " -> " + to + ": " + reason)
{
}
