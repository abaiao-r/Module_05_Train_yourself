/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
#define NODE_HPP

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "Edge.hpp"

class Node
{
  private:
	static std::unordered_set<std::string> &_existingNames();
	std::string _name;
	std::vector<Edge> _edges;

  public:
	Node() = delete;
	explicit Node(const std::string &name);
	Node(const Node &) = delete;
	Node(Node &&) = delete;
	Node &operator=(const Node &) = delete;
	Node &operator=(Node &&) = delete;
	~Node();

	void addEdge(std::weak_ptr<Node> node, double distance, double speedLimit);
	const std::string &getName() const;
	const std::vector<Edge> &getEdges() const;

	/* ---- Exceptions ---- */
	class SelfEdgeException : public std::runtime_error
	{
	  public:
		explicit SelfEdgeException(const std::string &nodeName);
	};

	class EdgeAlreadyExistsException : public std::runtime_error
	{
	  public:
		EdgeAlreadyExistsException(const std::string &from,
								   const std::string &to);
	};

	class InvalidNodeException : public std::runtime_error
	{
	  public:
		explicit InvalidNodeException(const std::string &nodeName);
	};

	class InvalidEdgeException : public std::runtime_error
	{
	  public:
		InvalidEdgeException(const std::string &from, const std::string &to,
							 const std::string &reason);
	};
};

#endif
