/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IPathfinding.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:22:01 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IPATHFINDING_HPP
#define IPATHFINDING_HPP

#include <memory>
#include <string>
#include <vector>

class Node;
class RailNetwork;

/**
 * How edge weights are computed during pathfinding.
 *   Distance — minimise total kilometres
 *   Time     — minimise total travel time (distance / speed_limit)
 */
enum class PathWeightMode
{
	Distance,
	Time
};

class IPathfinding
{
  public:
	virtual ~IPathfinding() = default;

	virtual std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network,
		PathWeightMode mode = PathWeightMode::Distance) const = 0;
};

#endif
