/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraPathfinding.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DijkstraPathfinding.hpp"

DijkstraPathfinding::DijkstraPathfinding() {}
DijkstraPathfinding::DijkstraPathfinding(const DijkstraPathfinding &) {}
DijkstraPathfinding &DijkstraPathfinding::operator=(
	const DijkstraPathfinding &)
{
	return *this;
}
DijkstraPathfinding::~DijkstraPathfinding() {}

std::vector<std::shared_ptr<Node>> DijkstraPathfinding::findPath(
	std::shared_ptr<Node> /*start*/, std::shared_ptr<Node> /*end*/)
{
	// TODO: implement Dijkstra algorithm
	return {};
}
