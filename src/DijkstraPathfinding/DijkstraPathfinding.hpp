/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraPathfinding.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 17:55:55 by abaiao-r         ###   ########.fr       */
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

	/** Realistic-aware pathfinding — adds penalty for occupied segments
		and for segments/nodes prone to costly random events. */
	std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network, PathWeightMode mode,
		const SegmentOccupancy &occupancy,
		const SegmentClearTimes &clearTimes = {},
		const SegmentEventRisk &eventRisk = {}) const override;

	/** Realistic penalty multiplier (seconds per train on segment).
		Fallback used when no clear-time estimate is available for a
		segment; otherwise the real estimated clear time is used. */
	static constexpr double CONGESTION_PENALTY = 120.0;
	/** Floor applied to a real clear-time estimate so an "almost clear"
		segment isn't treated as entirely free. */
	static constexpr double MIN_CONGESTION_PENALTY = 10.0;

	static double edgeWeight(const Edge &edge, PathWeightMode mode);
	static double edgeWeight(const Edge &edge, PathWeightMode mode,
							const std::string &from,
							const SegmentOccupancy &occupancy,
							const SegmentClearTimes &clearTimes = {},
							const SegmentEventRisk &eventRisk = {});
};

#endif
