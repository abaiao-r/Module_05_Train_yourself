/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 21:11:20 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 15:19:05 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Node.hpp"

std::unordered_set<std::string> Node::_existingNames;

// Parameterized constructor
Node::Node(const std::string &name) : _name(name)
{
	if (name.empty() || _existingNames.find(name) != _existingNames.end())
	{
		throw InvalidNodeException(name);
	}
	_existingNames.insert(name);
	addNodeToNetwork();
}

// Destructor
Node::~Node()
{
}

// Getters
const std::string &Node::getName() const
{
	return (this->_name);
}

const std::vector<std::shared_ptr<Edge>> &Node::getEdges() const
{
	return (this->_edges);
}

const std::vector<Event> &Node::getEvents() const
{
	return (this->_events);
}

// Add an edge to the node
void Node::addEdge(const Edge &edge)
{
	this->_edges.push_back(std::make_shared<Edge>(edge));
}

// Add an event to the node
void Node::addEvent(const Event &event)
{
	_events.push_back(event);
}

void Node::addNodeToNetwork()
{
	// This function cannot safely register 'this' as a shared_ptr<Node> from inside the constructor.
	// Instead, registration should be done externally, right after creating the shared_ptr<Node>:
	// Example usage:
	// auto node = std::make_shared<Node>("A");
	// RailNetwork::getInstance().addNode(node);
	// 
	// If you want to register from inside the class, Node must inherit from std::enable_shared_from_this<Node>
	// and you must create Node with std::make_shared<Node>(...).
	// Then you can use shared_from_this() here:
	// RailNetwork::getInstance().addNode(shared_from_this());
	//
	// Otherwise, leave this function empty or remove it, and always register nodes externally.
}

