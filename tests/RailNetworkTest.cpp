/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetworkTest.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RailNetwork.hpp"
#include "TestFramework.hpp"

int main()
{
	Test::TestSuite suite("RailNetwork");

	suite.run("addNode increases count", [](std::string &msg) {
		RailNetwork net;
		net.addNode("A");
		net.addNode("B");
		ASSERT_EQ(2u, net.nodeCount(), msg);
		return true;
	});

	suite.run("addNode throws on duplicate", [](std::string &msg) {
		RailNetwork net;
		net.addNode("A");
		ASSERT_THROWS(net.addNode("A"),
					  RailNetwork::DuplicateNodeException, msg);
		return true;
	});

	suite.run("findNode returns correct node", [](std::string &msg) {
		RailNetwork net;
		net.addNode("Station1");
		auto node = net.findNode("Station1");
		ASSERT_STR_EQ(std::string("Station1"), node->getName(), msg);
		return true;
	});

	suite.run("findNode throws for unknown", [](std::string &msg) {
		RailNetwork net;
		ASSERT_THROWS(net.findNode("Ghost"),
					  RailNetwork::NodeNotFoundException, msg);
		return true;
	});

	suite.run("addConnection creates bidirectional edges",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("X");
				  net.addNode("Y");
				  net.addConnection("X", "Y", 10.0, 100.0);
				  ASSERT_EQ(1u, net.getNeighbours("X").size(), msg);
				  ASSERT_EQ(1u, net.getNeighbours("Y").size(), msg);
				  return true;
			  });

	suite.run("addConnection throws on self-loop", [](std::string &msg) {
		RailNetwork net;
		net.addNode("S");
		ASSERT_THROWS(net.addConnection("S", "S", 5.0, 50.0),
					  std::invalid_argument, msg);
		return true;
	});

	suite.run("addConnection throws on duplicate",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("A");
				  net.addNode("B");
				  net.addConnection("A", "B", 10.0, 100.0);
				  ASSERT_THROWS(
					  net.addConnection("A", "B", 10.0, 100.0),
					  RailNetwork::DuplicateConnectionException, msg);
				  return true;
			  });

	suite.run("addConnection throws on unknown node",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("A");
				  ASSERT_THROWS(
					  net.addConnection("A", "Z", 10.0, 100.0),
					  RailNetwork::NodeNotFoundException, msg);
				  return true;
			  });

	suite.run("addConnection throws on bad distance",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("A");
				  net.addNode("B");
				  ASSERT_THROWS(
					  net.addConnection("A", "B", -1.0, 100.0),
					  std::invalid_argument, msg);
				  return true;
			  });

	suite.run("connection stores values correctly",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("P");
				  net.addNode("Q");
				  net.addConnection("P", "Q", 42.5, 200.0);
				  const auto &edges = net.getNeighbours("P");
				  ASSERT_TRUE(edges[0].distance == 42.5, msg);
				  ASSERT_TRUE(edges[0].speedLimit == 200.0, msg);
				  return true;
			  });

	suite.run("getNodeNames returns sorted names",
			  [](std::string &msg) {
				  RailNetwork net;
				  net.addNode("Charlie");
				  net.addNode("Alpha");
				  net.addNode("Bravo");
				  auto names = net.getNodeNames();
				  ASSERT_STR_EQ(std::string("Alpha"), names[0], msg);
				  ASSERT_STR_EQ(std::string("Bravo"), names[1], msg);
				  ASSERT_STR_EQ(std::string("Charlie"), names[2], msg);
				  return true;
			  });

	return suite.summarize();
}
