/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AStarPathfinding.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AStarPathfinding.hpp"

AStarPathfinding::AStarPathfinding() {}
AStarPathfinding::AStarPathfinding(const AStarPathfinding &) {}
AStarPathfinding &AStarPathfinding::operator=(const AStarPathfinding &)
{
	return *this;
}
AStarPathfinding::~AStarPathfinding() {}

std::vector<std::shared_ptr<Node>> AStarPathfinding::findPath(
	std::shared_ptr<Node> /*start*/, std::shared_ptr<Node> /*end*/)
{
	// TODO: implement A* algorithm
	return {};
}
