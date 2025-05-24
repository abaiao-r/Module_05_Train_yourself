/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Edge.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 12:17:36 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 14:23:15 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EDGE_HPP
#define EDGE_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "Event.hpp"
#include "Node.hpp"	 // Add this include for Node forward declaration and static lookup

class Edge
{
	private:
		std::string		   _node1;
		std::string		   _node2;
		size_t			   _distance;
		size_t			   _speedLimit;
		std::vector<Event> _events;	 // Store events by value, not shared_ptr

		void addEdgeToNodes();	// Helper to add this edge to both nodes

	public:
		Edge() = delete;  // Only allow parametric constructor
		Edge(const std::string &node1, const std::string &node2,
			 size_t distance, size_t speedLimit);
		Edge(const Edge &src) = delete;
		Edge(Edge &&src) = delete;
		Edge &operator=(const Edge &src) = delete;
		Edge &operator=(Edge &&src) = delete;
		~Edge();

		const std::string		  &getNode1() const;
		const std::string		  &getNode2() const;
		size_t					  getDistance() const;
		size_t					  getSpeedLimit() const;
		const std::vector<Event> &getEvents() const;
		void					  addEvent(const Event &event);
};

#endif
