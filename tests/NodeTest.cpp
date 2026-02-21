/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NodeTest.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Node.hpp"
#include "TestFramework.hpp"

int main()
{
	Test::TestSuite suite("Node");

	suite.run("constructor sets name", [](std::string &msg) {
		Node n("StationA");
		ASSERT_STR_EQ(std::string("StationA"), n.getName(), msg);
		return true;
	});

	suite.run("constructor throws on empty name", [](std::string &msg) {
		ASSERT_THROWS(Node(""), Node::InvalidNameException, msg);
		return true;
	});

	suite.run("copy constructor copies name", [](std::string &msg) {
		Node a("CopyTest");
		Node b(a);
		ASSERT_STR_EQ(a.getName(), b.getName(), msg);
		return true;
	});

	suite.run("assignment operator copies name", [](std::string &msg) {
		Node a("AssignA");
		Node b("AssignB");
		b = a;
		ASSERT_STR_EQ(std::string("AssignA"), b.getName(), msg);
		return true;
	});

	return suite.summarize();
}
