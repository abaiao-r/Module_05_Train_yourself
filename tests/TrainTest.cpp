/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TrainTest.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 03:22:12 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <memory>

#include "Node.hpp"
#include "TestFramework.hpp"
#include "Train.hpp"

int main()
{
	Test::TestSuite suite("Train");

	suite.run("constructor sets all fields", [](std::string &msg) {
		Train t("Express", 1.5, 1.2, "CityA", "CityB", 50400.0);
		ASSERT_STR_EQ(std::string("Express"), t.getName(), msg);
		ASSERT_TRUE(t.getMaxAcceleration() == 1.5, msg);
		ASSERT_TRUE(t.getMaxBrakingForce() == 1.2, msg);
		ASSERT_STR_EQ(std::string("CityA"), t.getDepartureStation(), msg);
		ASSERT_STR_EQ(std::string("CityB"), t.getArrivalStation(), msg);
		ASSERT_TRUE(t.getDepartureTime() == 50400.0, msg);
		ASSERT_TRUE(t.getStatus() == TrainStatus::Waiting, msg);
		return true;
	});

	suite.run("setPath and getPath work", [](std::string &msg) {
		Train t("T1", 1.0, 1.0, "A", "C", 0.0);
		auto a = std::make_shared<Node>("A");
		auto b = std::make_shared<Node>("B");
		auto c = std::make_shared<Node>("C");
		t.setPath({a, b, c});
		ASSERT_EQ(3u, t.getPath().size(), msg);
		ASSERT_EQ(0u, t.getPathIndex(), msg);
		return true;
	});

	suite.run("advanceToNextNode updates state", [](std::string &msg) {
		Train t("T2", 1.0, 1.0, "A", "C", 1000.0);
		auto a = std::make_shared<Node>("A");
		auto b = std::make_shared<Node>("B");
		auto c = std::make_shared<Node>("C");
		t.setPath({a, b, c});
		t.advanceToNextNode(500.0);
		ASSERT_EQ(1u, t.getPathIndex(), msg);
		ASSERT_TRUE(t.getCurrentTime() == 1500.0, msg);
		ASSERT_TRUE(t.getStatus() == TrainStatus::Running, msg);
		return true;
	});

	suite.run("advanceToNextNode marks arrived at end",
			  [](std::string &msg) {
				  Train t("T3", 1.0, 1.0, "A", "B", 0.0);
				  auto a = std::make_shared<Node>("A");
				  auto b = std::make_shared<Node>("B");
				  t.setPath({a, b});
				  t.advanceToNextNode(100.0);
				  ASSERT_TRUE(t.hasArrived(), msg);
				  return true;
			  });

	suite.run("applyDelay accumulates", [](std::string &msg) {
		Train t("T4", 1.0, 1.0, "A", "B", 0.0);
		t.applyDelay(60.0);
		t.applyDelay(120.0);
		ASSERT_TRUE(t.getTotalDelay() == 180.0, msg);
		ASSERT_TRUE(t.getCurrentTime() == 180.0, msg);
		return true;
	});

	suite.run("copy constructor works", [](std::string &msg) {
		Train original("Orig", 2.0, 1.5, "X", "Y", 3600.0);
		Train copy(original);
		ASSERT_STR_EQ(original.getName(), copy.getName(), msg);
		ASSERT_TRUE(copy.getDepartureTime() == 3600.0, msg);
		return true;
	});

	suite.run("statusToString returns correct strings",
			  [](std::string &msg) {
				  ASSERT_STR_EQ(std::string("Waiting"),
								Train::statusToString(TrainStatus::Waiting),
								msg);
				  ASSERT_STR_EQ(std::string("Arrived"),
								Train::statusToString(TrainStatus::Arrived),
								msg);
				  return true;
			  });

	suite.run("statusToString for Running and Delayed",
			  [](std::string &msg) {
				  ASSERT_STR_EQ(std::string("Running"),
								Train::statusToString(TrainStatus::Running),
								msg);
				  ASSERT_STR_EQ(std::string("Delayed"),
								Train::statusToString(TrainStatus::Delayed),
								msg);
				  return true;
			  });

	suite.run("getCurrentNodeName empty path returns empty",
			  [](std::string &msg) {
				  Train t("T", 1.0, 1.0, "A", "B", 0.0);
				  ASSERT_STR_EQ(std::string(""), t.getCurrentNodeName(), msg);
				  return true;
			  });

	suite.run("advanceToNextNode on empty path is safe",
			  [](std::string &msg) {
				  Train t("T", 1.0, 1.0, "A", "B", 0.0);
				  t.advanceToNextNode(100.0);
				  ASSERT_EQ(0u, t.getPathIndex(), msg);
				  return true;
			  });

	suite.run("hasArrived is false after construction",
			  [](std::string &msg) {
				  Train t("T", 1.0, 1.0, "A", "B", 0.0);
				  ASSERT_FALSE(t.hasArrived(), msg);
				  return true;
			  });

	return suite.summarize();
}
