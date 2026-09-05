/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CongestionTest.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 10:00:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 19:17:55 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <memory>
#include <string>
#include <unordered_map>

#include "DijkstraPathfinding.hpp"
#include "IPathfinding.hpp"
#include "Node.hpp"
#include "RailNetwork.hpp"
#include "TestFramework.hpp"

/**
 * Build a diamond network:
 *
 *       B
 *      / \
 *  A--+   +--D
 *      \ /
 *       C
 *
 * A-B 10 km 100 km/h   A-C 10 km 100 km/h
 * B-D 10 km 100 km/h   C-D 10 km 100 km/h
 *
 * In Solo mode both paths are equal (0.1 h each segment).
 * In Adaptive mode, occupying A->B should make A-C-D preferred.
 */
static RailNetwork makeDiamond()
{
	RailNetwork net;
	net.addNode("A");
	net.addNode("B");
	net.addNode("C");
	net.addNode("D");
	net.addConnection("A", "B", 10.0, 100.0);
	net.addConnection("A", "C", 10.0, 100.0);
	net.addConnection("B", "D", 10.0, 100.0);
	net.addConnection("C", "D", 10.0, 100.0);
	return net;
}

/**
 * Build a 3-route network:
 *
 *      B
 *     / \
 *  A--C---E
 *     \ /
 *      D
 *
 * A-B 10 / 100,  B-E 10 / 100
 * A-C 10 / 100,  C-E 10 / 100
 * A-D 10 / 100,  D-E 10 / 100
 */
static RailNetwork makeTriple()
{
	RailNetwork net;
	net.addNode("A");
	net.addNode("B");
	net.addNode("C");
	net.addNode("D");
	net.addNode("E");
	net.addConnection("A", "B", 10.0, 100.0);
	net.addConnection("A", "C", 10.0, 100.0);
	net.addConnection("A", "D", 10.0, 100.0);
	net.addConnection("B", "E", 10.0, 100.0);
	net.addConnection("C", "E", 10.0, 100.0);
	net.addConnection("D", "E", 10.0, 100.0);
	return net;
}

static std::string pathStr(const std::vector<std::shared_ptr<Node>> &path)
{
	std::string s;
	for (size_t i = 0; i < path.size(); i++)
	{
		if (i > 0)
			s += "->";
		s += path[i]->getName();
	}
	return s;
}

int main()
{
	Test::TestSuite suite("Adaptive");

	/* ── enum value exists ────────────────────────────────── */
	suite.run("PathWeightMode::Adaptive exists", [](std::string &msg) {
		PathWeightMode m = PathWeightMode::Adaptive;
		ASSERT_TRUE(m != PathWeightMode::Distance, msg);
		ASSERT_TRUE(m != PathWeightMode::Solo, msg);
		return true;
	});

	/* ── edgeWeight base cost matches Solo mode ──────────── */
	suite.run("Adaptive base cost equals Solo cost", [](std::string &msg) {
		RailNetwork net = makeDiamond();
		const auto &edges = net.getNeighbours("A");
		const auto &edge = edges[0]; // A->B or A->C

		double timeCost = DijkstraPathfinding::edgeWeight(
			edge, PathWeightMode::Solo);
		double congBase = DijkstraPathfinding::edgeWeight(
			edge, PathWeightMode::Adaptive);
		ASSERT_NEAR(timeCost, congBase, 1e-9, msg);
		return true;
	});

	/* ── empty occupancy == Solo mode path ───────────────── */
	suite.run("empty occupancy returns same path as Solo",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		DijkstraPathfinding dijk;
		SegmentOccupancy empty;

		auto timePath = dijk.findPath("A", "D", net,
									  PathWeightMode::Solo);
		auto congPath = dijk.findPath("A", "D", net,
									  PathWeightMode::Adaptive, empty);
		ASSERT_STR_EQ(pathStr(timePath), pathStr(congPath), msg);
		return true;
	});

	/* ── occupancy on A->B shifts route to A->C->D ──────── */
	suite.run("occupied segment is avoided",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		DijkstraPathfinding dijk;

		/* Without congestion: first path (deterministic via node order) */
		auto basePath = dijk.findPath("A", "D", net,
									  PathWeightMode::Solo);
		std::string baseVia = basePath[1]->getName(); // B or C

		/* Load occupancy on that first segment */
		SegmentOccupancy occ;
		occ["A->" + baseVia] = 3; // heavy congestion on the default route

		auto congPath = dijk.findPath("A", "D", net,
									  PathWeightMode::Adaptive, occ);
		ASSERT_EQ(3u, congPath.size(), msg);
		/* Via the OTHER intermediate node */
		std::string congVia = congPath[1]->getName();
		ASSERT_TRUE(congVia != baseVia,
					msg + " expected reroute from " + baseVia
						+ " but got " + congVia);
		return true;
	});

	/* ── penalty is additive per train ───────────────────── */
	suite.run("more trains on segment -> higher cost",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		const auto &edges = net.getNeighbours("A");
		/* Find the A->B edge */
		const Edge *abEdge = nullptr;
		for (const auto &e : edges)
		{
			if (e.destination->getName() == "B")
			{
				abEdge = &e;
				break;
			}
		}
		ASSERT_TRUE(abEdge != nullptr, msg);

		SegmentOccupancy occ1, occ2;
		occ1["A->B"] = 1;
		occ2["A->B"] = 5;

		double cost1 = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive, "A", occ1);
		double cost2 = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive, "A", occ2);
		double base = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive);

		ASSERT_TRUE(cost1 > base, msg);
		ASSERT_TRUE(cost2 > cost1, msg);
		ASSERT_NEAR(cost2 - cost1, 4.0 * 120.0 / 3600.0, 1e-9, msg);
		return true;
	});

	/* ── non-occupied segment has zero penalty ───────────── */
	suite.run("non-occupied segment has zero penalty",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		const auto &edges = net.getNeighbours("A");
		const Edge *acEdge = nullptr;
		for (const auto &e : edges)
		{
			if (e.destination->getName() == "C")
			{
				acEdge = &e;
				break;
			}
		}
		ASSERT_TRUE(acEdge != nullptr, msg);

		SegmentOccupancy occ;
		occ["A->B"] = 5; // only A->B is occupied

		double base = DijkstraPathfinding::edgeWeight(
			*acEdge, PathWeightMode::Adaptive);
		double withOcc = DijkstraPathfinding::edgeWeight(
			*acEdge, PathWeightMode::Adaptive, "A", occ);
		ASSERT_NEAR(base, withOcc, 1e-9, msg);
		return true;
	});

	/* ── congestion on all routes: picks least congested ── */
	suite.run("picks least congested route",
			  [](std::string &msg) {
		RailNetwork net = makeTriple();
		DijkstraPathfinding dijk;

		SegmentOccupancy occ;
		occ["A->B"] = 10;
		occ["A->C"] = 2;
		occ["A->D"] = 5;

		auto path = dijk.findPath("A", "E", net,
								  PathWeightMode::Adaptive, occ);
		ASSERT_EQ(3u, path.size(), msg);
		ASSERT_STR_EQ(std::string("C"), path[1]->getName(), msg);
		return true;
	});

	/* ── start == end returns single node ─────────────────── */
	suite.run("start equals end with occupancy",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		DijkstraPathfinding dijk;
		SegmentOccupancy occ;
		occ["A->B"] = 3;

		auto path = dijk.findPath("A", "A", net,
								  PathWeightMode::Adaptive, occ);
		ASSERT_EQ(1u, path.size(), msg);
		ASSERT_STR_EQ(std::string("A"), path[0]->getName(), msg);
		return true;
	});

	/* ── unreachable returns empty ────────────────────────── */
	suite.run("disconnected graph returns empty with occupancy",
			  [](std::string &msg) {
		RailNetwork net;
		net.addNode("X");
		net.addNode("Y");
		DijkstraPathfinding dijk;
		SegmentOccupancy occ;

		auto path = dijk.findPath("X", "Y", net,
								  PathWeightMode::Adaptive, occ);
		ASSERT_TRUE(path.empty(), msg);
		return true;
	});

	/* ── CONGESTION_PENALTY constant value ────────────────── */
	suite.run("CONGESTION_PENALTY is 120 seconds",
			  [](std::string &msg) {
		ASSERT_NEAR(120.0, DijkstraPathfinding::CONGESTION_PENALTY,
					  1e-9, msg);
		return true;
	});

	/* ── Non-Adaptive mode ignores occupancy ───────────── */
	suite.run("Distance mode ignores occupancy overload",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		DijkstraPathfinding dijk;

		SegmentOccupancy occ;
		occ["A->B"] = 100;

		/* Distance mode through the occupancy overload should still
		   delegate to the base findPath and ignore occupancy */
		auto distPath = dijk.findPath("A", "D", net,
									  PathWeightMode::Distance);
		auto distOccPath = dijk.findPath("A", "D", net,
										 PathWeightMode::Distance, occ);
		ASSERT_STR_EQ(pathStr(distPath), pathStr(distOccPath), msg);
		return true;
	});

	/* ── bidirectional occupancy keys are separate ────────── */
	suite.run("A->B and B->A are separate occupancy keys",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		const auto &edges = net.getNeighbours("A");
		const Edge *abEdge = nullptr;
		for (const auto &e : edges)
		{
			if (e.destination->getName() == "B")
			{
				abEdge = &e;
				break;
			}
		}
		ASSERT_TRUE(abEdge != nullptr, msg);

		SegmentOccupancy occ;
		occ["B->A"] = 5; // reverse direction

		/* A->B should NOT be penalised */
		double base = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive);
		double withOcc = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive, "A", occ);
		ASSERT_NEAR(base, withOcc, 1e-9, msg);
		return true;
	});

	/* ── time-aware penalty: real clear-time estimate is used ────── */
	suite.run("clear-time estimate overrides the flat penalty",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		const auto &edges = net.getNeighbours("A");
		const Edge *abEdge = nullptr;
		for (const auto &e : edges)
		{
			if (e.destination->getName() == "B")
			{
				abEdge = &e;
				break;
			}
		}
		ASSERT_TRUE(abEdge != nullptr, msg);

		SegmentOccupancy occ;
		occ["A->B"] = 1;
		SegmentClearTimes ct;
		ct["A->B"] = 500.0; // much bigger than the 120s flat constant

		double base = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive);
		double flatOnly = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive, "A", occ);
		double withClearTime = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive, "A", occ, ct);

		ASSERT_NEAR(flatOnly - base,
					DijkstraPathfinding::CONGESTION_PENALTY / 3600.0,
					1e-9, msg);
		ASSERT_NEAR(withClearTime - base, 500.0 / 3600.0, 1e-9, msg);
		ASSERT_TRUE(withClearTime > flatOnly, msg);
		return true;
	});

	/* ── time-aware penalty is floored, not treated as free ──────── */
	suite.run("clear-time estimate is floored at MIN_CONGESTION_PENALTY",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		const auto &edges = net.getNeighbours("A");
		const Edge *abEdge = nullptr;
		for (const auto &e : edges)
		{
			if (e.destination->getName() == "B")
			{
				abEdge = &e;
				break;
			}
		}
		ASSERT_TRUE(abEdge != nullptr, msg);

		SegmentOccupancy occ;
		occ["A->B"] = 1;
		SegmentClearTimes ct;
		ct["A->B"] = 1.0; // nearly cleared — should still cost >= floor

		double base = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive);
		double withClearTime = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive, "A", occ, ct);

		ASSERT_NEAR(withClearTime - base,
					DijkstraPathfinding::MIN_CONGESTION_PENALTY / 3600.0,
					1e-9, msg);
		return true;
	});

	/* ── missing clear-time entry falls back to the flat penalty ─── */
	suite.run("segment without a clear-time entry uses the flat penalty",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		const auto &edges = net.getNeighbours("A");
		const Edge *abEdge = nullptr;
		for (const auto &e : edges)
		{
			if (e.destination->getName() == "B")
			{
				abEdge = &e;
				break;
			}
		}
		ASSERT_TRUE(abEdge != nullptr, msg);

		SegmentOccupancy occ;
		occ["A->B"] = 2;
		SegmentClearTimes ct; // empty — no estimate for A->B

		double flatOnly = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive, "A", occ);
		double withEmptyClearTimes = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive, "A", occ, ct);

		ASSERT_NEAR(flatOnly, withEmptyClearTimes, 1e-9, msg);
		return true;
	});

	/* ── event risk adds cost even with zero occupancy ───────────── */
	suite.run("event risk penalises an edge with no current traffic",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		const auto &edges = net.getNeighbours("A");
		const Edge *abEdge = nullptr;
		for (const auto &e : edges)
		{
			if (e.destination->getName() == "B")
			{
				abEdge = &e;
				break;
			}
		}
		ASSERT_TRUE(abEdge != nullptr, msg);

		SegmentOccupancy occ; // empty — no traffic at all
		SegmentClearTimes ct;
		SegmentEventRisk risk;
		risk["A->B"] = 1800.0; // 30 minutes expected delay

		double base = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive);
		double withRisk = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Adaptive, "A", occ, ct, risk);

		ASSERT_NEAR(withRisk - base, 1800.0 / 3600.0, 1e-9, msg);
		return true;
	});

	/* ── findPath avoids a risky edge even before any train departs ── */
	suite.run("findPath reroutes around event risk with empty occupancy",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		DijkstraPathfinding dijk;

		auto basePath = dijk.findPath("A", "D", net,
									  PathWeightMode::Solo);
		std::string baseVia = basePath[1]->getName(); // B or C

		SegmentOccupancy occ; // no traffic
		SegmentClearTimes ct;
		SegmentEventRisk risk;
		risk["A->" + baseVia] = 3600.0; // huge risk on the default route

		auto adaptivePath = dijk.findPath("A", "D", net,
									   PathWeightMode::Adaptive, occ,
									   ct, risk);
		ASSERT_EQ(3u, adaptivePath.size(), msg);
		std::string adaptiveVia = adaptivePath[1]->getName();
		ASSERT_TRUE(adaptiveVia != baseVia,
					msg + " expected reroute from " + baseVia
						+ " but got " + adaptiveVia);
		return true;
	});

	/* ── missing event-risk entry has no effect ──────────────────── */
	suite.run("segment without an event-risk entry is unaffected",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		const auto &edges = net.getNeighbours("A");
		const Edge *acEdge = nullptr;
		for (const auto &e : edges)
		{
			if (e.destination->getName() == "C")
			{
				acEdge = &e;
				break;
			}
		}
		ASSERT_TRUE(acEdge != nullptr, msg);

		SegmentOccupancy occ;
		SegmentClearTimes ct;
		SegmentEventRisk risk;
		risk["A->B"] = 3600.0; // only A->B carries risk

		double base = DijkstraPathfinding::edgeWeight(
			*acEdge, PathWeightMode::Adaptive);
		double withRisk = DijkstraPathfinding::edgeWeight(
			*acEdge, PathWeightMode::Adaptive, "A", occ, ct, risk);

		ASSERT_NEAR(base, withRisk, 1e-9, msg);
		return true;
	});

	return suite.summarize();
}
