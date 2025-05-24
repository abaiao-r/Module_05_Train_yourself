/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Edge.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 12:18:41 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 15:23:02 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Edge.hpp"

#include "../includes/Node.hpp"

// Parametric constructor
Edge::Edge(const std::string &node1, const std::string &node2, size_t distance,
		   size_t speedLimit)
	: _node1(node1), _node2(node2), _distance(distance), _speedLimit(speedLimit)
{
	addEdgeToNodes();
}

// Destructor
Edge::~Edge()
{
	// Custom cleanup logic can be added here if needed in the future
}

// Getters
const std::string &Edge::getNode1() const
{
	return (this->_node1);
}

const std::string &Edge::getNode2() const
{
	return (this->_node2);
}

size_t Edge::getDistance() const
{
	return (this->_distance);
}

size_t Edge::getSpeedLimit() const
{
	return (this->_speedLimit);
}

const std::vector<Event> &Edge::getEvents() const
{
	return (this->_events);
}

// Add event to edge
void Edge::addEvent(const Event &event)
{
	this->_events.push_back(event);
}

void Edge::addEdgeToNodes()
{
	// This approach does not make sense if Node stores edges as
	// std::shared_ptr<Edge> and you are passing *this (an Edge by value) to
	// Node::addEdge. This will create a new shared_ptr<Edge> for each node, so
	// each node will have a different copy of the edge. If you want the same
	// Edge instance to be shared, you must create a shared_ptr<Edge> first,
	// then add that same shared_ptr to both nodes.

	// The correct approach is to create the shared_ptr<Edge> outside, then add
	// it to both nodes: Example (not inside Edge): auto edgePtr =
	// std::make_shared<Edge>(...); node1->addEdge(edgePtr);
	// node2->addEdge(edgePtr);

	// If you want to keep this logic inside Edge, you need to redesign so that
	// Edge is created as a shared_ptr and passed to both nodes at construction
	// time. Otherwise, this function will always add different copies of the
	// edge to each node.

	// Recommendation: Remove addEdgeToNodes() from Edge and manage edge sharing
	// externally.
}
