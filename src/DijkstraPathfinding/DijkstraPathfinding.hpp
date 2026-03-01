/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraPathfinding.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/03/01 18:39:51 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIJKSTRAPATHFINDING_HPP
#define DIJKSTRAPATHFINDING_HPP

#include "IPathfinding.hpp"

struct Edge;

class DijkstraPathfinding : public IPathfinding
{
  public:
	DijkstraPathfinding();
	DijkstraPathfinding(const DijkstraPathfinding &);
	DijkstraPathfinding &operator=(const DijkstraPathfinding &);
	~DijkstraPathfinding() override;

	std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network,
		PathWeightMode mode = PathWeightMode::Distance) const override;

	/** Congestion-aware pathfinding — adds penalty for occupied segments. */
	std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network, PathWeightMode mode,
		const SegmentOccupancy &occupancy) const override;

	/** Congestion penalty multiplier (seconds per train on segment). */
	static constexpr double CONGESTION_PENALTY = 120.0;

	static double edgeWeight(const Edge &edge, PathWeightMode mode);
	static double edgeWeight(const Edge &edge, PathWeightMode mode,
							const std::string &from,
							const SegmentOccupancy &occupancy);
};

#endif
