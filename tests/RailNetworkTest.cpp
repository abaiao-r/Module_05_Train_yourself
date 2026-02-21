/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetworkTest.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RailNetwork.hpp"

#include <memory>

#include "TestFramework.hpp"

int main()
{
	Test::TestSuite suite("RailNetwork");

	suite.run("getInstance returns singleton", [](std::string &msg) {
		auto &net1 = RailNetwork::getInstance();
		auto &net2 = RailNetwork::getInstance();
		ASSERT_TRUE(&net1 == &net2, msg);
		return true;
	});

	suite.run("addNode adds node to network", [](std::string &msg) {
		auto &net = RailNetwork::getInstance();
		auto node = std::make_shared<Node>("TestStation1");
		net.addNode(node);
		auto nodes = net.getNodes();
		bool found = false;
		for (const auto &n : nodes)
		{
			if (n->getName() == "TestStation1")
			{
				found = true;
				break;
			}
		}
		ASSERT_TRUE(found, msg);
		return true;
	});

	suite.run("addConnection creates bidirectional edge", [](std::string &msg) {
		auto &net = RailNetwork::getInstance();
		auto a = std::make_shared<Node>("BiDirA");
		auto b = std::make_shared<Node>("BiDirB");
		net.addConnection(a, b, 150.0, 80.0);

		auto neighborsA = net.getNeighbours(a);
		auto neighborsB = net.getNeighbours(b);
		ASSERT_EQ(1u, neighborsA.size(), msg);
		ASSERT_EQ(1u, neighborsB.size(), msg);

		auto nOfA = neighborsA[0].node.lock();
		auto nOfB = neighborsB[0].node.lock();
		ASSERT_TRUE(nOfA != nullptr, msg);
		ASSERT_TRUE(nOfB != nullptr, msg);
		ASSERT_STR_EQ(std::string("BiDirB"), nOfA->getName(), msg);
		ASSERT_STR_EQ(std::string("BiDirA"), nOfB->getName(), msg);
		return true;
	});

	suite.run("addConnection throws on self-connection", [](std::string &msg) {
		auto &net = RailNetwork::getInstance();
		auto node = std::make_shared<Node>("SelfStation");
		ASSERT_THROWS(net.addConnection(node, node, 100.0, 50.0),
					  Node::SelfEdgeException, msg);
		return true;
	});

	suite.run("addConnection throws on duplicate", [](std::string &msg) {
		auto &net = RailNetwork::getInstance();
		auto c = std::make_shared<Node>("DupC");
		auto d = std::make_shared<Node>("DupD");
		net.addConnection(c, d, 200.0, 100.0);
		ASSERT_THROWS(net.addConnection(c, d, 200.0, 100.0),
					  RailNetwork::ConnectionAlreadyExistsException, msg);
		return true;
	});

	suite.run("getNeighbours throws for unknown node", [](std::string &msg) {
		auto &net = RailNetwork::getInstance();
		auto orphan = std::make_shared<Node>("Orphan");
		ASSERT_THROWS(net.getNeighbours(orphan), std::runtime_error, msg);
		return true;
	});

	suite.run("connection stores distance and speed", [](std::string &msg) {
		auto &net = RailNetwork::getInstance();
		auto e = std::make_shared<Node>("ValE");
		auto f = std::make_shared<Node>("ValF");
		net.addConnection(e, f, 300.0, 120.0);
		auto neighbors = net.getNeighbours(e);
		ASSERT_TRUE(neighbors[0].distance == 300.0, msg);
		ASSERT_TRUE(neighbors[0].speedLimit == 120.0, msg);
		return true;
	});

	return suite.summarize();
}
