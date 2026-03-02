/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CongestionTest.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 10:00:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/03/01 18:39:51 by ctw03933         ###   ########.fr       */
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
 * In Time mode both paths are equal (0.1 h each segment).
 * In Congestion mode, occupying A->B should make A-C-D preferred.
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
	Test::TestSuite suite("Congestion");

	/* ── enum value exists ────────────────────────────────── */
	suite.run("PathWeightMode::Congestion exists", [](std::string &msg) {
		PathWeightMode m = PathWeightMode::Congestion;
		ASSERT_TRUE(m != PathWeightMode::Distance, msg);
		ASSERT_TRUE(m != PathWeightMode::Time, msg);
		return true;
	});

	/* ── edgeWeight base cost matches Time mode ──────────── */
	suite.run("Congestion base cost equals Time cost", [](std::string &msg) {
		RailNetwork net = makeDiamond();
		const auto &edges = net.getNeighbours("A");
		const auto &edge = edges[0]; // A->B or A->C

		double timeCost = DijkstraPathfinding::edgeWeight(
			edge, PathWeightMode::Time);
		double congBase = DijkstraPathfinding::edgeWeight(
			edge, PathWeightMode::Congestion);
		ASSERT_NEAR(timeCost, congBase, 1e-9, msg);
		return true;
	});

	/* ── empty occupancy == Time mode path ───────────────── */
	suite.run("empty occupancy returns same path as Time",
			  [](std::string &msg) {
		RailNetwork net = makeDiamond();
		DijkstraPathfinding dijk;
		SegmentOccupancy empty;

		auto timePath = dijk.findPath("A", "D", net,
									  PathWeightMode::Time);
		auto congPath = dijk.findPath("A", "D", net,
									  PathWeightMode::Congestion, empty);
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
									  PathWeightMode::Time);
		std::string baseVia = basePath[1]->getName(); // B or C

		/* Load occupancy on that first segment */
		SegmentOccupancy occ;
		occ["A->" + baseVia] = 3; // heavy congestion on the default route

		auto congPath = dijk.findPath("A", "D", net,
									  PathWeightMode::Congestion, occ);
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
			*abEdge, PathWeightMode::Congestion, "A", occ1);
		double cost2 = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Congestion, "A", occ2);
		double base = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Congestion);

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
			*acEdge, PathWeightMode::Congestion);
		double withOcc = DijkstraPathfinding::edgeWeight(
			*acEdge, PathWeightMode::Congestion, "A", occ);
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
								  PathWeightMode::Congestion, occ);
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
								  PathWeightMode::Congestion, occ);
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
								  PathWeightMode::Congestion, occ);
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

	/* ── Non-Congestion mode ignores occupancy ───────────── */
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
			*abEdge, PathWeightMode::Congestion);
		double withOcc = DijkstraPathfinding::edgeWeight(
			*abEdge, PathWeightMode::Congestion, "A", occ);
		ASSERT_NEAR(base, withOcc, 1e-9, msg);
		return true;
	});

	return suite.summarize();
}
