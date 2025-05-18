/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 20:36:48 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/18 20:32:38 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
#define NODE_HPP

#include "Edge.hpp"
#include "Event.hpp"  // Add this include
#include "libraries.hpp"

class Node
{
	private:
		// Static set to keep track of existing node names
		static std::unordered_set<std::string> _existingNames;
		std::string							   _name;

		// Vector to store edges
		std::vector<Edge>  _edges;
		std::vector<std::shared_ptr<Event>> _events;	 // Node owns its events

	public:
		Node() = delete;
		Node(std::string name);
		Node(const Node &src) = delete;
		Node(Node &&src) = delete;
		~Node();
		Node &operator=(const Node &src) = delete;
		Node &operator=(Node &&src) = delete;

		void			   addEdge(std::weak_ptr<Node> node, size_t distance,
								   size_t speedLimit);
		const std::string &getName() const;
		const std::vector<Edge>	&getEdges() const;
		void addEvent(const std::shared_ptr<Event>& event);
		void addEvent(const std::string &eventName, float probability, std::chrono::seconds duration);
		const std::vector<std::shared_ptr<Event>> &getEvents() const;

		// Custom exception class
		class SelfEdgeException : public std::runtime_error
		{
			public:
				SelfEdgeException(const std::string &nodeName)
					: std::runtime_error("Error: Node '" + nodeName
										 + "' cannot point to itself.")
				{
				}
		};

		// Custom exception class for existing edge
		class EdgeAlreadyExistsException : public std::runtime_error
		{
			public:
				EdgeAlreadyExistsException(const std::string &nodeName,
										   const std::string &targetNodeName)
					: std::runtime_error("Error: Edge already exists for node: "
										 + nodeName + " to " + targetNodeName)
				{
				}
		};

		// Custom exception class for duplicate node or invalid node name
		class InvalidNodeException : public std::runtime_error
		{
			public:
				InvalidNodeException(const std::string &nodeName,
									 bool				isNull = false)
					: std::runtime_error(
						isNull
							? "Error: Node name is null."
							: (nodeName.empty()
								   ? "Error: Node name cannot be empty."
								   : "Error: Node already exists: " + nodeName))
				{
				}
		};

		// Custom exception class for invalid edge parameters (distance, speed,
		// etc.)
		class InvalidEdgeException : public std::runtime_error
		{
			public:
				InvalidEdgeException(const std::string &nodeName,
									 const std::string &targetNodeName,
									 const std::string &reason)
					: std::runtime_error("Error: Invalid edge from node '"
										 + nodeName + "' to '" + targetNodeName
										 + "': " + reason)
				{
				}
		};

		// Custom exception class for duplicate event name
		class InvalidEventException : public std::runtime_error
		{
			public:
				InvalidEventException(const std::string &nodeName,
									  const std::string &eventName)
					: std::runtime_error("Duplicate event name '" + eventName
										 + "' for node: " + nodeName)
				{
				}
		};
};

#endif
