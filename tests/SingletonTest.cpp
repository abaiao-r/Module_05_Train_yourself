/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SingletonTest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Singleton.hpp"
#include "TestFramework.hpp"

class TestSingleton : public Singleton<TestSingleton>
{
  public:
	int value = 42;
};

int main()
{
	Test::TestSuite suite("Singleton");

	suite.run("getInstance returns valid reference", [](std::string &msg) {
		TestSingleton &instance = TestSingleton::getInstance();
		ASSERT_EQ(42, instance.value, msg);
		return true;
	});

	suite.run("getInstance returns same instance", [](std::string &msg) {
		TestSingleton &inst1 = TestSingleton::getInstance();
		TestSingleton &inst2 = TestSingleton::getInstance();
		ASSERT_TRUE(&inst1 == &inst2, msg);
		return true;
	});

	suite.run("modifications persist across calls", [](std::string &msg) {
		TestSingleton::getInstance().value = 100;
		ASSERT_EQ(100, TestSingleton::getInstance().value, msg);
		return true;
	});

	return suite.summarize();
}
