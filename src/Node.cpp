/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 21:11:20 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 13:44:15 by andrefranci      ###   ########.fr       */
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

const std::vector<Edge> &Node::getEdges() const
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
	_edges.push_back(edge);
}

// Add an event to the node
void Node::addEvent(const Event &event)
{
	_events.push_back(event);
}

// main to test the Node class

/* int main()
{
	try
	{
		// Create nodes
		std::shared_ptr<Node> nodeA = std::make_shared<Node>("A");
		std::shared_ptr<Node> nodeB = std::make_shared<Node>("B");
		std::shared_ptr<Node> nodeC = std::make_shared<Node>("C");

		// Add edges
		nodeA->addEdge(nodeB, 10);
		nodeA->addEdge(nodeC, 20);

		// Attempt to add a duplicate edge
		try
		{
			nodeA->addEdge(nodeB, 10);
		}
		catch (const Node::EdgeAlreadyExistsException &e)
		{
			std::cerr << e.what() << std::endl;
		}

		// Attempt to add a self-edge
		try
		{
			nodeA->addEdge(nodeA, 5);
		}
		catch (const Node::SelfEdgeException &e)
		{
			std::cerr << e.what() << std::endl;
		}

		// Attempt to create a node with a duplicate name
		try
		{
			std::shared_ptr<Node> nodeDuplicate = std::make_shared<Node>("A");
		}
		catch (const Node::DuplicateNodeException &e)
		{
			std::cerr << e.what() << std::endl;
		}

		// Print node names and edges
		std::cout << "Node " << nodeA->getName()
				  << " has edges to:" << std::endl;
		for (const auto &edge : nodeA->getEdges())
		{
			if (auto targetNode = edge.first.lock())
			{
				std::cout << "  Node " << targetNode->getName()
						  << " with distance " << edge.second << std::endl;
			}
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
} */
