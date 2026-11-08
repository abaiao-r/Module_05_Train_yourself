#ifndef COOPERATIVECOST_HPP
#define COOPERATIVECOST_HPP

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "IPathfinding.hpp"
#include "RailNetwork.hpp"
#include "ReservationTable.hpp"
#include "SegmentTiming.hpp"

/**
 * Congestion toll for the cooperative planner: the extra seconds a train
 * would lose (or cause) by entering a rail at `tEnter` given what is
 * already reserved. It is added to the free-flow time of that rail, so
 * Dijkstra only ever sees a heavier edge, never a forbidden one.
 *
 *   toll(lane) = wait until no opposite train is on the lane
 *              + FOLLOW_PENALTY_SEC * same-way trains already on the lane
 *   toll       = min over lanes   (a free parallel lane gives 0)
 */
/* Measured on the simulator: each extra same-way train on a rail costs
   the followers roughly this much (convoys are cheap, detours are not). */
constexpr double FOLLOW_PENALTY_SEC = 20.0;

/** Index of the cheapest lane of `edge` and its toll. Ties: lowest lane. */
inline std::pair<int, double> bestLane(const ReservationTable &table,
									   const std::string &from,
									   const Edge &edge, double tEnter,
									   bool lanesEnabled)
{
	const std::string &to = edge.destination->getName();
	int lanes = lanesEnabled ? std::max(1, edge.numLines) : 1;
	std::pair<int, double> best{0, std::numeric_limits<double>::infinity()};
	for (int lane = 0; lane < lanes; lane++)
	{
		double dur = freeFlowSeconds(edge, lane);
		double start = table.earliestFree(from, to, lane, tEnter, dur);
		double toll = (start - tEnter)
					  + FOLLOW_PENALTY_SEC
							* table.sameDirectionCount(from, to, lane, start,
													   start + dur);
		if (toll < best.second)
			best = {lane, toll};
	}
	return best;
}

inline double segmentToll(const ReservationTable &table,
						  const std::string &from, const Edge &edge,
						  double tEnter, bool lanesEnabled)
{
	return bestLane(table, from, edge, tEnter, lanesEnabled).second;
}

/**
 * Per-directed-segment extra seconds ("from->to" keys, like event risk)
 * for a train that leaves `origin` at `departure`. Entry times are the
 * free-flow arrival times, so the toll is priced where the train would
 * actually be. `baseRisk` (event risk) is carried through unchanged.
 */
inline SegmentEventRisk tollMap(const ReservationTable &table,
								const RailNetwork &network,
								const std::string &origin, double departure,
								double dwellSeconds, bool lanesEnabled,
								const SegmentEventRisk &baseRisk = {})
{
	const double INF = std::numeric_limits<double>::infinity();
	std::unordered_map<std::string, double> at;
	for (const auto &n : network.getNodeNames())
		at[n] = INF;
	at[origin] = departure;

	using Item = std::pair<double, std::string>;
	std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pq;
	pq.push({departure, origin});
	while (!pq.empty())
	{
		auto [t, u] = pq.top();
		pq.pop();
		if (t > at[u])
			continue;
		double leave = (u == origin) ? t : t + dwellSeconds;
		for (const Edge &e : network.getNeighbours(u))
		{
			const std::string &v = e.destination->getName();
			double arrive = leave + freeFlowSeconds(e);
			if (arrive < at[v])
			{
				at[v] = arrive;
				pq.push({arrive, v});
			}
		}
	}

	SegmentEventRisk out = baseRisk;
	for (const auto &u : network.getNodeNames())
	{
		if (at[u] == INF)
			continue;
		double leave = (u == origin) ? at[u] : at[u] + dwellSeconds;
		for (const Edge &e : network.getNeighbours(u))
		{
			double toll = segmentToll(table, u, e, leave, lanesEnabled);
			if (toll > 0.0)
				out[u + "->" + e.destination->getName()] += toll;
		}
	}
	return out;
}

#endif
