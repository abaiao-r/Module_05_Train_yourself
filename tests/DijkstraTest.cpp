/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraTest.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 14:26:44 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <memory>

#include "DijkstraPathfinding.hpp"
#include "Node.hpp"
#include "RailNetwork.hpp"
#include "TestFramework.hpp"

int main()
{
	Test::TestSuite suite("Dijkstra");

	suite.run("finds direct path", [](std::string &msg) {
		RailNetwork net;
		net.addNode("A");
		net.addNode("B");
		net.addConnection("A", "B", 10.0, 100.0);

		DijkstraPathfinding dijk;
		auto path = dijk.findPath("A", "B", net);
		ASSERT_EQ(2u, path.size(), msg);
		ASSERT_STR_EQ(std::string("A"), path[0]->getName(), msg);
		ASSERT_STR_EQ(std::string("B"), path[1]->getName(), msg);
		return true;
	});

	suite.run("finds shortest of two paths", [](std::string &msg) {
		RailNetwork net;
		net.addNode("A");
		net.addNode("B");
		net.addNode("C");
		net.addConnection("A", "B", 5.0, 100.0);
		net.addConnection("B", "C", 5.0, 100.0);
		net.addConnection("A", "C", 20.0, 100.0);

		DijkstraPathfinding dijk;
		auto path = dijk.findPath("A", "C", net);
		ASSERT_EQ(3u, path.size(), msg);
		ASSERT_STR_EQ(std::string("A"), path[0]->getName(), msg);
		ASSERT_STR_EQ(std::string("B"), path[1]->getName(), msg);
		ASSERT_STR_EQ(std::string("C"), path[2]->getName(), msg);
		return true;
	});

	suite.run("returns empty for disconnected graph",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("A");
				  net.addNode("Z");

				  DijkstraPathfinding dijk;
				  auto path = dijk.findPath("A", "Z", net);
				  ASSERT_TRUE(path.empty(), msg);
				  return true;
			  });

	suite.run("handles multi-hop network", [](std::string &msg) {
		RailNetwork net;
		net.addNode("A");
		net.addNode("B");
		net.addNode("C");
		net.addNode("D");
		net.addConnection("A", "B", 1.0, 100.0);
		net.addConnection("B", "C", 1.0, 100.0);
		net.addConnection("C", "D", 1.0, 100.0);
		net.addConnection("A", "D", 10.0, 100.0);

		DijkstraPathfinding dijk;
		auto path = dijk.findPath("A", "D", net);
		ASSERT_EQ(4u, path.size(), msg);
		return true;
	});

	suite.run("path from node to itself", [](std::string &msg) {
		RailNetwork net;
		net.addNode("A");

		DijkstraPathfinding dijk;
		auto path = dijk.findPath("A", "A", net);
		ASSERT_EQ(1u, path.size(), msg);
		ASSERT_STR_EQ(std::string("A"), path[0]->getName(), msg);
		return true;
	});

	suite.run("throws when start node not in network",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("B");
				  DijkstraPathfinding dijk;
				  ASSERT_THROWS(dijk.findPath("Ghost", "B", net),
								RailNetwork::NodeNotFoundException, msg);
				  return true;
			  });

	suite.run("throws when end node not in network",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("A");
				  DijkstraPathfinding dijk;
				  ASSERT_THROWS(dijk.findPath("A", "Ghost", net),
								RailNetwork::NodeNotFoundException, msg);
				  return true;
			  });

	/* ---- PathWeightMode::Time tests ---- */

	suite.run("time mode prefers faster route over shorter",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("A");
				  net.addNode("B");
				  net.addNode("C");
				  // Short but slow: A->B->C = 10km @ 50 km/h => 0.2h
				  net.addConnection("A", "B", 5.0, 50.0);
				  net.addConnection("B", "C", 5.0, 50.0);
				  // Long but fast: A->C = 20km @ 300 km/h => 0.067h
				  net.addConnection("A", "C", 20.0, 300.0);

				  DijkstraPathfinding dijk;
				  // Distance mode: A->B->C (10 km < 20 km)
				  auto distPath = dijk.findPath("A", "C", net,
												PathWeightMode::Distance);
				  ASSERT_EQ(3u, distPath.size(), msg);
				  ASSERT_STR_EQ(std::string("B"),
								distPath[1]->getName(), msg);

				  // Time mode: A->C direct (0.067h < 0.2h)
				  auto timePath = dijk.findPath("A", "C", net,
												PathWeightMode::Time);
				  ASSERT_EQ(2u, timePath.size(), msg);
				  ASSERT_STR_EQ(std::string("A"),
								timePath[0]->getName(), msg);
				  ASSERT_STR_EQ(std::string("C"),
								timePath[1]->getName(), msg);
				  return true;
			  });

	suite.run("time mode same result when speeds equal",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("A");
				  net.addNode("B");
				  net.addNode("C");
				  net.addConnection("A", "B", 5.0, 100.0);
				  net.addConnection("B", "C", 5.0, 100.0);
				  net.addConnection("A", "C", 20.0, 100.0);

				  DijkstraPathfinding dijk;
				  auto distP = dijk.findPath("A", "C", net,
											 PathWeightMode::Distance);
				  auto timeP = dijk.findPath("A", "C", net,
											 PathWeightMode::Time);
				  ASSERT_EQ(distP.size(), timeP.size(), msg);
				  return true;
			  });

	suite.run("time mode multi-hop picks fastest total",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("A");
				  net.addNode("B");
				  net.addNode("C");
				  net.addNode("D");
				  // Fast route: A->B->D  = 10/200 + 10/200 = 0.1h
				  net.addConnection("A", "B", 10.0, 200.0);
				  net.addConnection("B", "D", 10.0, 200.0);
				  // Slow route: A->C->D  = 5/50 + 5/50  = 0.2h
				  net.addConnection("A", "C", 5.0, 50.0);
				  net.addConnection("C", "D", 5.0, 50.0);

				  DijkstraPathfinding dijk;
				  // Distance mode: A->C->D (10 km < 20 km)
				  auto distP = dijk.findPath("A", "D", net,
											 PathWeightMode::Distance);
				  ASSERT_EQ(3u, distP.size(), msg);
				  ASSERT_STR_EQ(std::string("C"),
								distP[1]->getName(), msg);

				  // Time mode: A->B->D (0.1h < 0.2h)
				  auto timeP = dijk.findPath("A", "D", net,
											 PathWeightMode::Time);
				  ASSERT_EQ(3u, timeP.size(), msg);
				  ASSERT_STR_EQ(std::string("B"),
								timeP[1]->getName(), msg);
				  return true;
			  });

	suite.run("default mode is Distance", [](std::string &msg) {
		RailNetwork net;
		net.addNode("A");
		net.addNode("B");
		net.addNode("C");
		// Short but slow
		net.addConnection("A", "B", 5.0, 50.0);
		net.addConnection("B", "C", 5.0, 50.0);
		// Long but fast
		net.addConnection("A", "C", 20.0, 300.0);

		DijkstraPathfinding dijk;
		// No mode arg → defaults to Distance → A->B->C
		auto path = dijk.findPath("A", "C", net);
		ASSERT_EQ(3u, path.size(), msg);
		ASSERT_STR_EQ(std::string("B"), path[1]->getName(), msg);
		return true;
	});

	return suite.summarize();
}
