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

	return suite.summarize();
}
