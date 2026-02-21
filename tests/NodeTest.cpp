/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeTest.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Node.hpp"

#include <memory>

#include "TestFramework.hpp"

int main()
{
	Test::TestSuite suite("Node");

	suite.run("constructor sets name", [](std::string &msg) {
		auto node = std::make_shared<Node>("StationA");
		ASSERT_STR_EQ(std::string("StationA"), node->getName(), msg);
		return true;
	});

	suite.run("constructor throws on empty name", [](std::string &msg) {
		ASSERT_THROWS(std::make_shared<Node>(""), Node::InvalidNodeException,
					  msg);
		return true;
	});

	suite.run("constructor throws on duplicate name", [](std::string &msg) {
		auto n1 = std::make_shared<Node>("DupNode");
		ASSERT_THROWS(std::make_shared<Node>("DupNode"),
					  Node::InvalidNodeException, msg);
		(void)n1;
		return true;
	});

	suite.run("addEdge adds edge correctly", [](std::string &msg) {
		auto a = std::make_shared<Node>("EdgeA");
		auto b = std::make_shared<Node>("EdgeB");
		a->addEdge(b, 100.0, 60.0);
		auto &edges = a->getEdges();
		ASSERT_EQ(1u, edges.size(), msg);
		ASSERT_TRUE(edges[0].distance == 100.0, msg);
		ASSERT_TRUE(edges[0].speedLimit == 60.0, msg);
		return true;
	});

	suite.run("addEdge throws on self-edge", [](std::string &msg) {
		auto node = std::make_shared<Node>("SelfNode");
		ASSERT_THROWS(node->addEdge(node, 10.0, 50.0),
					  Node::SelfEdgeException, msg);
		return true;
	});

	suite.run("addEdge throws on duplicate edge", [](std::string &msg) {
		auto x = std::make_shared<Node>("NodeX");
		auto y = std::make_shared<Node>("NodeY");
		x->addEdge(y, 50.0, 40.0);
		ASSERT_THROWS(x->addEdge(y, 50.0, 40.0),
					  Node::EdgeAlreadyExistsException, msg);
		return true;
	});

	suite.run("addEdge throws on zero distance", [](std::string &msg) {
		auto a = std::make_shared<Node>("ZeroDistA");
		auto b = std::make_shared<Node>("ZeroDistB");
		ASSERT_THROWS(a->addEdge(b, 0.0, 50.0), Node::InvalidEdgeException,
					  msg);
		return true;
	});

	suite.run("addEdge throws on negative speed", [](std::string &msg) {
		auto a = std::make_shared<Node>("NegSpdA");
		auto b = std::make_shared<Node>("NegSpdB");
		ASSERT_THROWS(a->addEdge(b, 10.0, -5.0), Node::InvalidEdgeException,
					  msg);
		return true;
	});

	suite.run("getEdges empty for new node", [](std::string &msg) {
		auto node = std::make_shared<Node>("EmptyNode");
		ASSERT_EQ(0u, node->getEdges().size(), msg);
		return true;
	});

	suite.run("multiple edges can be added", [](std::string &msg) {
		auto c = std::make_shared<Node>("Center");
		auto n1 = std::make_shared<Node>("Sat1");
		auto n2 = std::make_shared<Node>("Sat2");
		auto n3 = std::make_shared<Node>("Sat3");
		c->addEdge(n1, 10.0, 30.0);
		c->addEdge(n2, 20.0, 40.0);
		c->addEdge(n3, 30.0, 50.0);
		ASSERT_EQ(3u, c->getEdges().size(), msg);
		return true;
	});

	suite.run("destructor frees name for reuse", [](std::string &msg) {
		{
			auto tmp = std::make_shared<Node>("TempNode");
		}  // tmp destroyed here
		auto reused = std::make_shared<Node>("TempNode");
		ASSERT_STR_EQ(std::string("TempNode"), reused->getName(), msg);
		return true;
	});

	return suite.summarize();
}
