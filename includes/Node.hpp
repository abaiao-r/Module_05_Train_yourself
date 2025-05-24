/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 20:36:48 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 15:14:49 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
#define NODE_HPP

#include "Edge.hpp"
#include "Event.hpp"
#include "libraries.hpp"

class Node
{
	private:
		static std::unordered_set<std::string>
					_existingNames;	 // Keep for uniqueness
		std::string _name;
		std::vector<std::shared_ptr<Edge>> _edges;
		std::vector<Event>				   _events;

		void addNodeToNetwork();  // Helper to add this node to the network

	public:
		Node() = delete;
		explicit Node(const std::string &name);
		Node(const Node &src) = delete;
		Node(Node &&src) = delete;
		~Node();
		Node &operator=(const Node &src) = delete;
		Node &operator=(Node &&src) = delete;

		const std::string						  &getName() const;
		const std::vector<std::shared_ptr<Edge>> &getEdges() const;
		const std::vector<Event>				 &getEvents() const;

		void addEdge(const Edge &edge);
		void addEvent(const Event &event);

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
};

#endif
