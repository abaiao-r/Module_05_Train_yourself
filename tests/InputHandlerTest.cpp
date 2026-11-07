/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandlerTest.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "InputHandler.hpp"
#include "RailNetwork.hpp"
#include "TestFramework.hpp"
#include <fstream>

static const std::string NET = "input/railNetworkPrintFolder/";
static const std::string TRN = "input/trainPrintFolder/";
static const std::string GOOD_NET = NET + "railNetworkPrintGood.txt";
static const std::string GOOD_TRN = TRN + "trainPrintGood.txt";

int main()
{
	Test::TestSuite suite("InputHandler");


	auto writeFile = [](const std::string &path, const std::string &c) {
		std::ofstream(path) << c;
	};
	const std::string baseNet = "Node A\nNode B\nNode C\nRail A B 5 100\n";
	const std::string okTrain = "T 80 0.05 356 30 A B 14h10 00h10\n";

	suite.run("rejects trailing junk on rail line", [&](std::string &msg) {
		writeFile("/tmp/strict_n1.txt", baseNet + "Rail B C 5 100 junk\n");
		writeFile("/tmp/strict_t.txt", okTrain);
		ASSERT_THROWS(InputHandler::loadData("/tmp/strict_n1.txt",
											 "/tmp/strict_t.txt"),
					  std::exception, msg);
		return true;
	});

	suite.run("rejects malformed time suffix", [&](std::string &msg) {
		writeFile("/tmp/strict_n2.txt", baseNet);
		writeFile("/tmp/strict_t2.txt",
				  "T 80 0.05 356 30 A B 14h10x 00h10\n");
		ASSERT_THROWS(InputHandler::loadData("/tmp/strict_n2.txt",
											 "/tmp/strict_t2.txt"),
					  std::exception, msg);
		return true;
	});

	suite.run("rejects rail event on nonexistent rail", [&](std::string &msg) {
		writeFile("/tmp/strict_n3.txt",
				  baseNet + "Event Storm 0.5 1h A C\n");
		writeFile("/tmp/strict_t3.txt", okTrain);
		ASSERT_THROWS(InputHandler::loadData("/tmp/strict_n3.txt",
											 "/tmp/strict_t3.txt"),
					  std::exception, msg);
		return true;
	});

	suite.run("duplicate train detected across time spellings",
			  [&](std::string &msg) {
		writeFile("/tmp/strict_dn.txt", baseNet);
		writeFile("/tmp/strict_dt.txt",
				  "T 80 0.05 356 30 A B 8h00 00h10\n"
				  "T 80 0.05 356 30 A B 08h00 00h10\n");
		ASSERT_THROWS(InputHandler::loadData("/tmp/strict_dn.txt",
											 "/tmp/strict_dt.txt"),
					  std::exception, msg);
		return true;
	});


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

	suite.run("lines option: disabled by default, carried when enabled",
			  [](std::string &msg) {
				  auto off = InputHandler::loadData(GOOD_NET, GOOD_TRN);
				  ASSERT_FALSE(off.lines.enabled, msg);
				  LinesOptions on;
				  on.enabled = true;
				  auto data = InputHandler::loadData(GOOD_NET, GOOD_TRN, on);
				  ASSERT_TRUE(data.lines.enabled, msg);
				  return true;
			  });

	LinesOptions on;
	on.enabled = true;
	auto loadNet = [&](const std::string &rails, const LinesOptions &o) {
		writeFile("/tmp/lines_n.txt", "Node A\nNode B\nNode C\n" + rails);
		writeFile("/tmp/lines_t.txt", okTrain);
		return InputHandler::loadData("/tmp/lines_n.txt",
									  "/tmp/lines_t.txt", o);
	};
	auto errorOf = [&](const std::string &rails, const LinesOptions &o) {
		try
		{
			loadNet(rails, o);
		}
		catch (const InputHandler::ParseException &e)
		{
			return std::string(e.what());
		}
		return std::string("<no error>");
	};

	suite.run("lines on: Rail with count 3 gives an edge with 3 lines",
			  [&](std::string &msg) {
				  auto d = loadNet("Rail A B 10 100 3\n", on);
				  const auto &e = d.network.getNeighbours("A")[0];
				  ASSERT_EQ(3, e.numLines, msg);
				  ASSERT_NEAR(10.0, e.distance, 1e-9, msg);
				  ASSERT_EQ(3, d.network.getNeighbours("B")[0].numLines, msg);
				  return true;
			  });

	suite.run("lines on: Rail without count has exactly 1 line",
			  [&](std::string &msg) {
				  auto d = loadNet("Rail A B 10 100\n", on);
				  ASSERT_EQ(1, d.network.getNeighbours("A")[0].numLines, msg);
				  return true;
			  });

	suite.run("lines on: bad counts name the offending line and value",
			  [&](std::string &msg) {
				  for (const char *bad : {"0", "-1", "1.5", "abc", "2x",
										  "99999999999"})
				  {
					  std::string err = errorOf(
						  std::string("Rail A B 10 100 ") + bad + "\n", on);
					  ASSERT_TRUE(err.find("lines_n.txt:4:") != std::string::npos,
								  msg);
					  ASSERT_TRUE(err.find(std::string("Invalid line count '")
										   + bad + "'") != std::string::npos,
								  msg);
				  }
				  return true;
			  });

	suite.run("lines on: extra token after count is still rejected",
			  [&](std::string &msg) {
				  std::string err = errorOf("Rail A B 10 100 2 junk\n", on);
				  ASSERT_TRUE(err.find("Unexpected trailing content")
							  != std::string::npos, msg);
				  return true;
			  });

	suite.run("lines off: count token errors exactly as before",
			  [&](std::string &msg) {
				  std::string err = errorOf("Rail A B 10 100 3\n", {});
				  ASSERT_TRUE(err.find("lines_n.txt:4: Unexpected trailing content")
							  != std::string::npos, msg);
				  return true;
			  });

	suite.run("lines on: duplicate pair still throws",
			  [&](std::string &msg) {
				  std::string err = errorOf(
					  "Rail A B 10 100 2\nRail B A 12 90 3\n", on);
				  ASSERT_TRUE(err.find("lines_n.txt:5:") != std::string::npos,
							  msg);
				  return true;
			  });

	suite.run("sample parallel-lines file loads only with lines on",
			  [&](std::string &msg) {
				  const std::string f = NET + "railNetworkParallelLines.txt";
				  ASSERT_THROWS(InputHandler::loadData(f, GOOD_TRN),
								InputHandler::ParseException, msg);
				  RailNetwork n;
				  std::vector<Event> ev;
				  InputHandler::parseNetworkFile(f, n, ev, on);
				  ASSERT_EQ(2, n.getNeighbours("CityA")[0].numLines, msg);
				  ASSERT_EQ(1, n.getNeighbours("CityC")[0].numLines, msg);
				  return true;
			  });

	return suite.summarize();
}
