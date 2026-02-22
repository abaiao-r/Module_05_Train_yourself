/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandlerTest.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/22 13:18:37 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "InputHandler.hpp"
#include "RailNetwork.hpp"
#include "TestFramework.hpp"

static const std::string NET = "input/railNetworkPrintFolder/";
static const std::string TRN = "input/trainPrintFolder/";
static const std::string GOOD_NET = NET + "railNetworkPrintGood.txt";
static const std::string GOOD_TRN = TRN + "trainPrintGood.txt";

int main()
{
	Test::TestSuite suite("InputHandler");

	/* ── Valid input tests ── */

	suite.run("loads rail network file", [](std::string &msg) {
		auto data = InputHandler::loadData(GOOD_NET, GOOD_TRN);
		ASSERT_EQ(10u, data.network.nodeCount(), msg);
		return true;
	});

	suite.run("loads all trains", [](std::string &msg) {
		auto data = InputHandler::loadData(GOOD_NET, GOOD_TRN);
		ASSERT_EQ(3u, data.trains.size(), msg);
		return true;
	});

	suite.run("loads events including quoted names and rail events",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(GOOD_NET, GOOD_TRN);
				  ASSERT_EQ(6u, data.events.size(), msg);
				  /* Last event is a rail segment event */
				  bool hasRailEvent = false;
				  for (const auto &ev : data.events)
				  {
					  if (ev.isRailEvent())
						  hasRailEvent = true;
				  }
				  ASSERT_TRUE(hasRailEvent, msg);
				  return true;
			  });

	suite.run("train has correct departure time",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(GOOD_NET, GOOD_TRN);
				  ASSERT_TRUE(data.trains[0]->getDepartureTime() == 51000.0,
							  msg);
				  return true;
			  });

	suite.run("train has correct weight and friction",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(GOOD_NET, GOOD_TRN);
				  ASSERT_TRUE(data.trains[0]->getWeight() == 80.0, msg);
				  ASSERT_TRUE(
					  data.trains[0]->getFrictionCoefficient() == 0.05,
					  msg);
				  return true;
			  });

	suite.run("train has correct stop duration",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(GOOD_NET, GOOD_TRN);
				  ASSERT_TRUE(
					  data.trains[0]->getStopDuration() == 600.0, msg);
				  return true;
			  });

	suite.run("network has correct connections", [](std::string &msg) {
		auto data = InputHandler::loadData(GOOD_NET, GOOD_TRN);
		auto &edges = data.network.getNeighbours("CityA");
		ASSERT_TRUE(edges.size() >= 2, msg);
		return true;
	});

	/* ── Missing file tests ── */

	suite.run("throws on missing file", [](std::string &msg) {
		ASSERT_THROWS(
			InputHandler::loadData("nonexistent.txt", "also_missing.txt"),
			InputHandler::ParseException, msg);
		return true;
	});

	suite.run("throws on missing rail file only",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData("nonexistent.txt", GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on missing train file only",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(GOOD_NET, "missing.txt"),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	/* ── Bad network input tests ── */

	suite.run("throws on unknown keyword",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkBadKeyword.txt", GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on missing node name",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkMissingNodeName.txt", GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on non-numeric rail distance",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkBadRail.txt", GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on negative rail distance",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkNegativeDistance.txt", GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on zero speed limit",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkZeroSpeed.txt", GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on self-loop rail",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkSelfLoop.txt", GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on duplicate node",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkDuplicateNode.txt", GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on duplicate rail",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkDuplicateRail.txt", GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on rail with unknown node",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkUnknownNode.txt", GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	/* ── Bad event input tests ── */

	suite.run("throws on negative event duration",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkBadEventDuration.txt",
						  GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on unknown event duration unit",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkBadEventUnit.txt",
						  GOOD_TRN),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	/* ── Bad train input tests ── */

	suite.run("throws on invalid time (25h00)",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  GOOD_NET,
						  TRN + "trainPrintBadTime.txt"),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on incomplete train line",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  GOOD_NET,
						  TRN + "trainPrintIncomplete.txt"),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("throws on non-numeric time format",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  GOOD_NET,
						  TRN + "trainPrintBadTimeFormat.txt"),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	/* ── Edge case valid inputs ── */

	suite.run("empty network file throws",
			  [](std::string &msg) {
				  ASSERT_THROWS(
					  InputHandler::loadData(
						  NET + "railNetworkEmpty.txt",
						  TRN + "trainPrintEmpty.txt"),
					  InputHandler::ParseException, msg);
				  return true;
			  });

	suite.run("minimal network (2 nodes, 1 rail)",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(
					  NET + "railNetworkMinimal.txt",
					  TRN + "trainPrintSingle.txt");
				  ASSERT_EQ(2u, data.network.nodeCount(), msg);
				  ASSERT_EQ(1u, data.trains.size(), msg);
				  return true;
			  });

	suite.run("midnight departure (00h00) parses correctly",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(
					  NET + "railNetworkMinimal.txt",
					  TRN + "trainPrintMidnight.txt");
				  ASSERT_TRUE(
					  data.trains[0]->getDepartureTime() == 0.0, msg);
				  return true;
			  });

	suite.run("late night departure (23h59) parses correctly",
			  [](std::string &msg) {
				  auto data = InputHandler::loadData(
					  NET + "railNetworkMinimal.txt",
					  TRN + "trainPrintLateNight.txt");
				  ASSERT_TRUE(
					  data.trains[0]->getDepartureTime() == 86340.0, msg);
				  return true;
			  });

	return suite.summarize();
}
