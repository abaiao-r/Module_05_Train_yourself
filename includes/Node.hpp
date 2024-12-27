/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 20:36:48 by andrefranci       #+#    #+#             */
/*   Updated: 2024/10/13 23:11:14 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
#define NODE_HPP

#include "libraries.hpp"

class Node
{
	private:
		// Static set to keep track of existing node names
		static std::unordered_set<std::string> _existingNames;
		std::string							   _name;
		// PAIR OF NODE AND DISTANCE
		std::vector<std::pair<std::weak_ptr<Node>, size_t>> _edges;

	public:
		Node() = delete;
		Node(std::string name);
		Node(const Node &src) = delete;
		Node(Node &&src) = delete;
		~Node();
		Node &operator=(const Node &src) = delete;
		Node &operator=(Node &&src) = delete;

		void			   addEdge(std::weak_ptr<Node> node, size_t distance);
		const std::string &getName() const;
		const std::vector<std::pair<std::weak_ptr<Node>, size_t>> &getEdges()
			const;

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

		// Custom exception class for duplicate node
		class DuplicateNodeException : public std::runtime_error
		{
			public:
				DuplicateNodeException(const std::string &nodeName)
					: std::runtime_error("Error: Node already exists: "
										 + nodeName)
				{
				}
		};
};

#endif
