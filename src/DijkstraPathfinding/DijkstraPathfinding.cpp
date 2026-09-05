/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraPathfinding.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 17:55:55 by abaiao-r         ###   ########.fr       */
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
	if (mode == PathWeightMode::Solo || mode == PathWeightMode::Realistic)
		return edge.distance / edge.speedLimit; // hours
	return edge.distance;                       // km
}

/* ---- Edge weight with congestion + event-risk penalty ---- */
double DijkstraPathfinding::edgeWeight(const Edge &edge,
									   PathWeightMode mode,
									   const std::string &from,
									   const SegmentOccupancy &occupancy,
									   const SegmentClearTimes &clearTimes,
									   const SegmentEventRisk &eventRisk)
{
	double base = edgeWeight(edge, mode);
	if (mode == PathWeightMode::Realistic)
	{
		std::string key = from + "->" + edge.destination->getName();
		auto it = occupancy.find(key);
		if (it != occupancy.end() && it->second > 0)
		{
			/* Prefer a real estimated clear time over the flat constant —
			   ties the pathfinding cost to the actual delay a train would
			   suffer being capped behind the occupant(s), rather than a
			   fixed guess. Floor it so an almost-cleared segment isn't
			   treated as entirely free. */
			auto ct = clearTimes.find(key);
			double penaltySec = (ct != clearTimes.end())
				? std::max(ct->second, MIN_CONGESTION_PENALTY)
				: CONGESTION_PENALTY * it->second;
			base += penaltySec / 3600.0; // hours
		}

		/* Event risk is static (independent of current traffic), so it
		   applies unconditionally — even before any train has departed. */
		auto er = eventRisk.find(key);
		if (er != eventRisk.end())
			base += er->second / 3600.0; // hours
	}
	return base;
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

/* ---- Realistic-aware pathfinding ---- */
std::vector<std::shared_ptr<Node>> DijkstraPathfinding::findPath(
	const std::string &start, const std::string &end,
	const RailNetwork &network, PathWeightMode mode,
	const SegmentOccupancy &occupancy,
	const SegmentClearTimes &clearTimes,
	const SegmentEventRisk &eventRisk) const
{
	/* Event risk is static (present even before any train has departed),
	   so don't bail out to the plain overload just because occupancy is
	   still empty — only skip if there's truly nothing to weigh. */
	if (mode != PathWeightMode::Realistic
		|| (occupancy.empty() && eventRisk.empty()))
		return findPath(start, end, network, mode);

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
			double newDist = dist[u]
							 + edgeWeight(edge, mode, u, occupancy, clearTimes,
										 eventRisk);
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
