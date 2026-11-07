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
#include <vector>

class Node;

struct Edge
{
	std::shared_ptr<Node> destination;
	double distance;    // km
	double speedLimit;  // km/h
	int numLines = 1;   // parallel lines (bonus --lines); all share distance
	std::vector<double> lineSpeeds; // optional per-line km/h; empty = speedLimit

	double lineSpeed(int line) const
	{
		if (line < 0 || static_cast<size_t>(line) >= lineSpeeds.size())
			return speedLimit;
		return lineSpeeds[line];
	}

	/* Orthodox Canonical Form */
	Edge() = default;
	Edge(std::shared_ptr<Node> dest, double dist, double speed,
		 int lines = 1, std::vector<double> speeds = {})
		: destination(std::move(dest)), distance(dist), speedLimit(speed),
		  numLines(lines), lineSpeeds(std::move(speeds)) {}
	Edge(const Edge &other) = default;
	Edge &operator=(const Edge &other) = default;
	~Edge() = default;
};

#endif
