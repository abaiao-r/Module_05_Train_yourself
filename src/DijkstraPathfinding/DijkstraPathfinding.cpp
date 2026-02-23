/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraPathfinding.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DijkstraPathfinding.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <unordered_map>

#include "Edge.hpp"
#include "Node.hpp"
#include "RailNetwork.hpp"

/* ---- Canonical form ---- */
DijkstraPathfinding::DijkstraPathfinding() {}
DijkstraPathfinding::DijkstraPathfinding(const DijkstraPathfinding &) {}

DijkstraPathfinding &DijkstraPathfinding::operator=(
	const DijkstraPathfinding &)
{
	return *this;
}

DijkstraPathfinding::~DijkstraPathfinding() {}

/* ---- Edge weight helper ---- */
double DijkstraPathfinding::edgeWeight(const Edge &edge,
									   PathWeightMode mode)
{
	if (mode == PathWeightMode::Time)
		return edge.distance / edge.speedLimit; // hours
	return edge.distance;                       // km
}

/* ---- Algorithm ---- */
std::vector<std::shared_ptr<Node>> DijkstraPathfinding::findPath(
	const std::string &start, const std::string &end,
	const RailNetwork &network, PathWeightMode mode) const
{
	// Validate that both nodes exist in the network
	network.findNode(start);
	network.findNode(end);

	if (start == end)
		return {network.findNode(start)};

	using Pair = std::pair<double, std::string>;
	std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> pq;

	std::unordered_map<std::string, double> dist;
	std::unordered_map<std::string, std::string> prev;
	const double INF = std::numeric_limits<double>::infinity();

	for (const auto &name : network.getNodeNames())
		dist[name] = INF;

	dist[start] = 0.0;
	pq.push({0.0, start});

	while (!pq.empty())
	{
		auto [d, u] = pq.top();
		pq.pop();

		if (d > dist[u])
			continue;
		if (u == end)
			break;

		for (const auto &edge : network.getNeighbours(u))
		{
			const std::string &v = edge.destination->getName();
			double newDist = dist[u] + edgeWeight(edge, mode);
			if (newDist < dist[v])
			{
				dist[v] = newDist;
				prev[v] = u;
				pq.push({newDist, v});
			}
		}
	}

	std::vector<std::shared_ptr<Node>> path;
	if (dist[end] == INF)
		return path;

	std::string current = end;
	while (current != start)
	{
		path.push_back(network.findNode(current));
		current = prev[current];
	}
	path.push_back(network.findNode(start));
	std::reverse(path.begin(), path.end());
	return path;
}
