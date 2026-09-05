/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IPathfinding.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 19:15:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IPATHFINDING_HPP
#define IPATHFINDING_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Node;
class RailNetwork;

/**
 * How edge weights are computed during pathfinding.
 *   Distance   — minimise total kilometres
 *   Solo       — minimise total travel time (distance / speed_limit)
 *   Adaptive — minimise time + penalty for occupied segments
 */
enum class PathWeightMode
{
	Distance,
	Solo,
	Adaptive
};

/**
 * Number of trains currently on each directed segment.
 * Key = "from->to", value = train count.
 */
using SegmentOccupancy = std::unordered_map<std::string, int>;

/**
 * Estimated seconds until a segment's current occupant(s) fully clear it
 * (based on the slowest occupant's remaining distance / speed). Same
 * "from->to" keys as SegmentOccupancy. Used for a time-aware congestion
 * penalty instead of a flat per-train constant; empty/missing entries
 * fall back to the flat penalty.
 */
using SegmentClearTimes = std::unordered_map<std::string, double>;

/**
 * Expected delay (seconds) from random events bound to a segment or its
 * destination node — sum of probability * duration across all matching
 * events. Same "from->to" keys as SegmentOccupancy. Unlike congestion,
 * this is static for the network (independent of current traffic) and
 * applies from the very first route computed, not just reroutes.
 */
using SegmentEventRisk = std::unordered_map<std::string, double>;

class IPathfinding
{
  public:
	virtual ~IPathfinding() = default;

	virtual std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network,
		PathWeightMode mode = PathWeightMode::Distance) const = 0;

	/**
	 * Adaptive-aware overload.  Falls back to the base overload
	 * when mode != Adaptive or both occupancy and eventRisk are empty.
	 */
	virtual std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network, PathWeightMode mode,
		const SegmentOccupancy &occupancy,
		const SegmentClearTimes &clearTimes = {},
		const SegmentEventRisk &eventRisk = {}) const
	{
		(void)occupancy;
		(void)clearTimes;
		(void)eventRisk;
		return findPath(start, end, network, mode);
	}
};

#endif
