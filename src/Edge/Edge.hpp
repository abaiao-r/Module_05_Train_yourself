/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Edge.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 12:26:07 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EDGE_HPP
#define EDGE_HPP

#include <memory>

class Node;

struct Edge
{
	std::shared_ptr<Node> destination;
	double distance;    // km
	double speedLimit;  // km/h

	/* Orthodox Canonical Form */
	Edge() = default;
	Edge(std::shared_ptr<Node> dest, double dist, double speed)
		: destination(std::move(dest)), distance(dist), speedLimit(speed) {}
	Edge(const Edge &other) = default;
	Edge &operator=(const Edge &other) = default;
	~Edge() = default;
};

#endif
