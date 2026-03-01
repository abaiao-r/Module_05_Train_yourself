/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IPathfinding.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/03/01 18:28:59 by ctw03933         ###   ########.fr       */
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
 *   Time       — minimise total travel time (distance / speed_limit)
 *   Congestion — minimise time + penalty for occupied segments
 */
enum class PathWeightMode
{
	Distance,
	Time,
	Congestion
};

/**
 * Number of trains currently on each directed segment.
 * Key = "from->to", value = train count.
 */
using SegmentOccupancy = std::unordered_map<std::string, int>;

class IPathfinding
{
  public:
	virtual ~IPathfinding() = default;

	virtual std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network,
		PathWeightMode mode = PathWeightMode::Distance) const = 0;

	/**
	 * Congestion-aware overload.  Falls back to the base overload
	 * when mode != Congestion or occupancy is empty.
	 */
	virtual std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network, PathWeightMode mode,
		const SegmentOccupancy &occupancy) const
	{
		(void)occupancy;
		return findPath(start, end, network, mode);
	}
};

#endif
