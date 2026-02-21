/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraPathfinding.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r           #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DijkstraPathfinding.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <unordered_map>

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

/* ---- Algorithm ---- */
std::vector<std::shared_ptr<Node>> DijkstraPathfinding::findPath(
	const std::string &start, const std::string &end,
	const RailNetwork &network) const
{
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
			double newDist = dist[u] + edge.distance;
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
