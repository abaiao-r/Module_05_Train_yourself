/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TrainFactoryTest.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 03:24:01 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TestFramework.hpp"
#include "TrainFactory.hpp"

int main()
{
	Test::TestSuite suite("TrainFactory");

	suite.run("creates valid train", [](std::string &msg) {
		auto t = TrainFactory::createTrain("Express", 1.5, 1.2, "A", "B",
										   3600.0);
		ASSERT_TRUE(t != nullptr, msg);
		ASSERT_STR_EQ(std::string("Express"), t->getName(), msg);
		ASSERT_TRUE(t->getMaxAcceleration() == 1.5, msg);
		ASSERT_TRUE(t->getMaxBrakingForce() == 1.2, msg);
		return true;
	});

	suite.run("throws on empty name", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("", 1.0, 1.0, "A", "B", 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on zero acceleration", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("T", 0.0, 1.0, "A", "B", 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on negative acceleration", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("T", -1.0, 1.0, "A", "B", 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on zero braking", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("T", 1.0, 0.0, "A", "B", 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on empty departure", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("T", 1.0, 1.0, "", "B", 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on same departure and arrival",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  TrainFactory::createTrain("T", 1.0, 1.0, "A", "A",
												0.0),
					  std::invalid_argument, msg);
				  return true;
			  });

	suite.run("throws on negative departure time",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  TrainFactory::createTrain("T", 1.0, 1.0, "A", "B",
												-100.0),
					  std::invalid_argument, msg);
				  return true;
			  });

	return suite.summarize();
}
