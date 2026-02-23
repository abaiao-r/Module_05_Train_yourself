/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TrainFactoryTest.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TestFramework.hpp"
#include "TrainFactory.hpp"

int main()
{
	Test::TestSuite suite("TrainFactory");

	TrainFactory::resetIdCounter();

	suite.run("creates valid train with auto ID", [](std::string &msg) {
		TrainFactory::resetIdCounter();
		auto t = TrainFactory::createTrain("Express", 80.0, 0.05, 356.0,
										   30.0, "A", "B", 3600.0, 600.0);
		ASSERT_TRUE(t != nullptr, msg);
		ASSERT_EQ(1, t->getId(), msg);
		ASSERT_STR_EQ(std::string("Express"), t->getName(), msg);
		ASSERT_TRUE(t->getWeight() == 80.0, msg);
		ASSERT_TRUE(t->getFrictionCoefficient() == 0.05, msg);
		ASSERT_TRUE(t->getMaxAccelForce() == 356.0, msg);
		ASSERT_TRUE(t->getMaxBrakeForce() == 30.0, msg);
		ASSERT_TRUE(t->getStopDuration() == 600.0, msg);
		return true;
	});

	suite.run("throws on empty name", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("", 80.0, 0.05, 356.0, 30.0,
									  "A", "B", 0.0, 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on zero weight", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("T", 0.0, 0.05, 356.0, 30.0,
									  "A", "B", 0.0, 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on negative friction", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("T", 80.0, -0.05, 356.0, 30.0,
									  "A", "B", 0.0, 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on zero acceleration force", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("T", 80.0, 0.05, 0.0, 30.0,
									  "A", "B", 0.0, 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on zero braking force", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("T", 80.0, 0.05, 356.0, 0.0,
									  "A", "B", 0.0, 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on empty departure", [](std::string &msg) {
		ASSERT_THROWS(
			TrainFactory::createTrain("T", 80.0, 0.05, 356.0, 30.0,
									  "", "B", 0.0, 0.0),
			std::invalid_argument, msg);
		return true;
	});

	suite.run("throws on same departure and arrival",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  TrainFactory::createTrain("T", 80.0, 0.05, 356.0,
												30.0, "A", "A", 0.0,
												0.0),
					  std::invalid_argument, msg);
				  return true;
			  });

	suite.run("throws on negative departure time",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  TrainFactory::createTrain("T", 80.0, 0.05, 356.0,
												30.0, "A", "B", -100.0,
												0.0),
					  std::invalid_argument, msg);
				  return true;
			  });

	suite.run("throws on negative stop duration",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  TrainFactory::createTrain("T", 80.0, 0.05, 356.0,
												30.0, "A", "B", 0.0,
												-1.0),
					  std::invalid_argument, msg);
				  return true;
			  });

	suite.run("IDs auto-increment across calls", [](std::string &msg) {
		TrainFactory::resetIdCounter();
		auto t1 = TrainFactory::createTrain("A", 80.0, 0.05, 356.0,
											30.0, "X", "Y", 0.0, 0.0);
		auto t2 = TrainFactory::createTrain("B", 60.0, 0.04, 300.0,
											25.0, "X", "Z", 0.0, 0.0);
		auto t3 = TrainFactory::createTrain("C", 50.0, 0.03, 200.0,
											20.0, "Y", "Z", 0.0, 0.0);
		ASSERT_EQ(1, t1->getId(), msg);
		ASSERT_EQ(2, t2->getId(), msg);
		ASSERT_EQ(3, t3->getId(), msg);
		return true;
	});

	suite.run("failed creation does not consume an ID", [](std::string &msg) {
		TrainFactory::resetIdCounter();
		auto t1 = TrainFactory::createTrain("OK", 80.0, 0.05, 356.0,
											30.0, "A", "B", 0.0, 0.0);
		ASSERT_EQ(1, t1->getId(), msg);
		// These should all throw and NOT increment _nextId
		try { TrainFactory::createTrain("", 80.0, 0.05, 356.0,
										30.0, "A", "B", 0.0, 0.0); } catch (...) {}
		try { TrainFactory::createTrain("T", 0.0, 0.05, 356.0,
										30.0, "A", "B", 0.0, 0.0); } catch (...) {}
		try { TrainFactory::createTrain("T", 80.0, -1.0, 356.0,
										30.0, "A", "B", 0.0, 0.0); } catch (...) {}
		try { TrainFactory::createTrain("T", 80.0, 0.05, 0.0,
										30.0, "A", "B", 0.0, 0.0); } catch (...) {}
		try { TrainFactory::createTrain("T", 80.0, 0.05, 356.0,
										0.0, "A", "B", 0.0, 0.0); } catch (...) {}
		try { TrainFactory::createTrain("T", 80.0, 0.05, 356.0,
										30.0, "A", "A", 0.0, 0.0); } catch (...) {}
		try { TrainFactory::createTrain("T", 80.0, 0.05, 356.0,
										30.0, "A", "B", -1.0, 0.0); } catch (...) {}
		try { TrainFactory::createTrain("T", 80.0, 0.05, 356.0,
										30.0, "A", "B", 0.0, -1.0); } catch (...) {}
		// Next valid creation should be ID 2, not 10
		auto t2 = TrainFactory::createTrain("OK2", 80.0, 0.05, 356.0,
											30.0, "A", "B", 0.0, 0.0);
		ASSERT_EQ(2, t2->getId(), msg);
		return true;
	});

	suite.run("resetIdCounter mid-sequence restarts from 1",
			  [](std::string &msg) {
				  TrainFactory::resetIdCounter();
				  auto t1 = TrainFactory::createTrain("A", 80.0, 0.05,
													  356.0, 30.0, "X",
													  "Y", 0.0, 0.0);
				  auto t2 = TrainFactory::createTrain("B", 80.0, 0.05,
													  356.0, 30.0, "X",
													  "Y", 0.0, 0.0);
				  ASSERT_EQ(1, t1->getId(), msg);
				  ASSERT_EQ(2, t2->getId(), msg);
				  TrainFactory::resetIdCounter();
				  auto t3 = TrainFactory::createTrain("C", 80.0, 0.05,
													  356.0, 30.0, "X",
													  "Y", 0.0, 0.0);
				  ASSERT_EQ(1, t3->getId(), msg); // restarts at 1
				  return true;
			  });

	return suite.summarize();
}
