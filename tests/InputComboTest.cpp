/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputComboTest.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/01 00:30:00 by ctw03933          #+#    #+#             */
/*   Updated: 2026/03/01 15:45:29 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * Input-combination tests — runs ./bin/train_yourself as a subprocess against every
 * input file pair.  Valid pairs must exit 0; bad inputs must exit non-zero.
 * Compiled with -fsanitize=address so ASan catches any leaks or UB.
 */

#include <array>
#include <cstdio>
#include <string>

#include "TestFramework.hpp"

static const std::string BIN = "./bin/train_yourself";
static const std::string NET = "input/railNetworkPrintFolder/";
static const std::string TRN = "input/trainPrintFolder/";

/* ---- Helper ---- */

static int runExit(const std::string &cmd)
{
	std::string full = cmd + " >/dev/null 2>&1";
	int status = std::system(full.c_str());
#ifdef _WIN32
	return status;
#else
	return WEXITSTATUS(status);
#endif
}

static bool expectOk(const std::string &net, const std::string &trn,
					 const std::string &flags, std::string &msg)
{
	std::string cmd = BIN + " " + NET + net + " " + TRN + trn;
	if (!flags.empty())
		cmd += " " + flags;
	int ec = runExit(cmd);
	ASSERT_EQ(0, ec, msg);
	return true;
}

static bool expectFail(const std::string &net, const std::string &trn,
					   std::string &msg)
{
	std::string cmd = BIN + " " + NET + net + " " + TRN + trn;
	int ec = runExit(cmd);
	ASSERT_TRUE(ec != 0, msg);
	return true;
}

/* ======================================================================== */

int main()
{
	Test::TestSuite suite("Input-Combo");

	/* ------------------------------------------------------------------ */
	/*  Valid pairs — must exit 0                                          */
	/* ------------------------------------------------------------------ */

	suite.run("Good network + Good trains",
			  [](std::string &m) {
				  return expectOk("railNetworkPrintGood.txt",
								  "trainPrintGood.txt", "", m);
			  });

	suite.run("Good network + Good trains --time",
			  [](std::string &m) {
				  return expectOk("railNetworkPrintGood.txt",
								  "trainPrintGood.txt", "--time", m);
			  });

	suite.run("Minimal network + Single train",
			  [](std::string &m) {
				  return expectOk("railNetworkMinimal.txt",
								  "trainPrintSingle.txt", "", m);
			  });

	suite.run("France network + France trains",
			  [](std::string &m) {
				  return expectOk("railNetworkFrance.txt",
								  "trainPrintFrance.txt", "", m);
			  });

	suite.run("Portugal network + Portugal trains",
			  [](std::string &m) {
				  return expectOk("railNetworkPortugal.txt",
								  "trainPrintPortugal.txt", "", m);
			  });

	suite.run("Europe network + Europe trains",
			  [](std::string &m) {
				  return expectOk("railNetworkEurope.txt",
								  "trainPrintEurope.txt", "", m);
			  });

	suite.run("Lisbon Metro network + Lisbon Metro trains",
			  [](std::string &m) {
				  return expectOk("railNetworkLisbonMetro.txt",
								  "trainPrintLisbonMetro.txt", "", m);
			  });

	suite.run("Paris Metro network + Paris Metro trains",
			  [](std::string &m) {
				  return expectOk("railNetworkParisMetro.txt",
								  "trainPrintParisMetro.txt", "", m);
			  });

	suite.run("Good network + LateNight trains",
			  [](std::string &m) {
				  return expectOk("railNetworkPrintGood.txt",
								  "trainPrintLateNight.txt", "", m);
			  });

	suite.run("Good network + Midnight trains",
			  [](std::string &m) {
				  return expectOk("railNetworkPrintGood.txt",
								  "trainPrintMidnight.txt", "", m);
			  });

	suite.run("Disconnected network + Unreachable trains",
			  [](std::string &m) {
				  return expectOk("railNetworkDisconnected.txt",
								  "trainPrintUnreachable.txt", "", m);
			  });

	suite.run("Good network + Unreachable trains",
			  [](std::string &m) {
				  return expectOk("railNetworkPrintGood.txt",
								  "trainPrintUnreachable.txt", "", m);
			  });

	suite.run("Good network + Empty trains",
			  [](std::string &m) {
				  return expectOk("railNetworkPrintGood.txt",
								  "trainPrintEmpty.txt", "", m);
			  });

	/* ------------------------------------------------------------------ */
	/*  Valid pairs with flags                                             */
	/* ------------------------------------------------------------------ */

	suite.run("France --time",
			  [](std::string &m) {
				  return expectOk("railNetworkFrance.txt",
								  "trainPrintFrance.txt", "--time", m);
			  });

	suite.run("Portugal --time",
			  [](std::string &m) {
				  return expectOk("railNetworkPortugal.txt",
								  "trainPrintPortugal.txt", "--time", m);
			  });

	suite.run("Europe --time",
			  [](std::string &m) {
				  return expectOk("railNetworkEurope.txt",
								  "trainPrintEurope.txt", "--time", m);
			  });

	suite.run("Good --runs 3",
			  [](std::string &m) {
				  return expectOk("railNetworkPrintGood.txt",
								  "trainPrintGood.txt", "--runs 3", m);
			  });

	suite.run("France --graph",
			  [](std::string &m) {
				  return expectOk("railNetworkFrance.txt",
								  "trainPrintFrance.txt",
								  "--graph /tmp/ci_combo_test.dot", m);
			  });

	suite.run("Portugal --time --graph",
			  [](std::string &m) {
				  return expectOk("railNetworkPortugal.txt",
								  "trainPrintPortugal.txt",
								  "--time --graph /tmp/ci_combo_test2.dot",
								  m);
			  });

	/* ------------------------------------------------------------------ */
	/*  Bad network files — must exit non-zero                             */
	/* ------------------------------------------------------------------ */

	suite.run("BAD: Empty network",
			  [](std::string &m) {
				  return expectFail("railNetworkEmpty.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: BadKeyword network",
			  [](std::string &m) {
				  return expectFail("railNetworkBadKeyword.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: BadRail network",
			  [](std::string &m) {
				  return expectFail("railNetworkBadRail.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: NegativeDistance network",
			  [](std::string &m) {
				  return expectFail("railNetworkNegativeDistance.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: ZeroSpeed network",
			  [](std::string &m) {
				  return expectFail("railNetworkZeroSpeed.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: SelfLoop network",
			  [](std::string &m) {
				  return expectFail("railNetworkSelfLoop.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: UnknownNode network",
			  [](std::string &m) {
				  return expectFail("railNetworkUnknownNode.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: DuplicateNode network",
			  [](std::string &m) {
				  return expectFail("railNetworkDuplicateNode.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: DuplicateRail network",
			  [](std::string &m) {
				  return expectFail("railNetworkDuplicateRail.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: MissingNodeName network",
			  [](std::string &m) {
				  return expectFail("railNetworkMissingNodeName.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: BadEventDuration network",
			  [](std::string &m) {
				  return expectFail("railNetworkBadEventDuration.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: BadEventUnit network",
			  [](std::string &m) {
				  return expectFail("railNetworkBadEventUnit.txt",
									"trainPrintGood.txt", m);
			  });

	suite.run("BAD: QuotedNames network",
			  [](std::string &m) {
				  return expectFail("railNetworkQuotedNames.txt",
									"trainPrintGood.txt", m);
			  });

	/* ------------------------------------------------------------------ */
	/*  Bad train files — must exit non-zero                               */
	/* ------------------------------------------------------------------ */

	suite.run("BAD: BadStation trains",
			  [](std::string &m) {
				  return expectFail("railNetworkPrintGood.txt",
									"trainPrintBadStation.txt", m);
			  });

	suite.run("BAD: BadTime trains",
			  [](std::string &m) {
				  return expectFail("railNetworkPrintGood.txt",
									"trainPrintBadTime.txt", m);
			  });

	suite.run("BAD: BadTimeFormat trains",
			  [](std::string &m) {
				  return expectFail("railNetworkPrintGood.txt",
									"trainPrintBadTimeFormat.txt", m);
			  });

	suite.run("BAD: Incomplete trains",
			  [](std::string &m) {
				  return expectFail("railNetworkPrintGood.txt",
									"trainPrintIncomplete.txt", m);
			  });

	return suite.summarize();
}
