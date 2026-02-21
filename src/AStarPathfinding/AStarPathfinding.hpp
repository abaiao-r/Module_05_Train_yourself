/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AStarPathfinding.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ASTARPATHFINDING_HPP
#define ASTARPATHFINDING_HPP

#include <memory>
#include <vector>

#include "IPathfindingAlgorithm.hpp"
#include "Node.hpp"

class AStarPathfinding : public IPathfindingAlgorithm
{
  public:
	AStarPathfinding();
	AStarPathfinding(const AStarPathfinding &src);
	AStarPathfinding &operator=(const AStarPathfinding &src);
	~AStarPathfinding() override;

	std::vector<std::shared_ptr<Node>> findPath(
		std::shared_ptr<Node> start, std::shared_ptr<Node> end) override;
};

#endif
