/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Edge.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 12:18:41 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 15:03:52 by andrefranci      ###   ########.fr       */
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
	// Find Node instances by name (requires Node::findByName implementation)
	Node *n1 = Node::findByName(this->_node1);
	Node *n2 = Node::findByName(this->_node2);
	if (!n1 || !n2)
		return;	 // Or throw

	// Check if this edge already exists in n1
	bool existsInN1 = false;
	for (const auto &e : n1->getEdges())
	{
		if (e.getNode1() == this->_node1 && e.getNode2() == this->_node2
			&& e.getDistance() == this->_distance
			&& e.getSpeedLimit() == this->_speedLimit)
		{
			existsInN1 = true;
			break;
		}
	}
	if (!existsInN1)
		n1->addEdge(*this);

	// Check if this edge already exists in n2
	bool existsInN2 = false;
	for (const auto &e : n2->getEdges())
	{
		if (e.getNode1() == this->_node1 && e.getNode2() == this->_node2
			&& e.getDistance() == this->_distance
			&& e.getSpeedLimit() == this->_speedLimit)
		{
			existsInN2 = true;
			break;
		}
	}
	if (!existsInN2)
		n2->addEdge(*this);
}
