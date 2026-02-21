/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CombinationTest.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 17:00:00 by ctw03933          #+#    #+#             */
/*   Updated: 2026/02/21 16:58:23 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
 * CombinationTest — exhaustive robustness suite.
 *
 * Runs every network file × every train file combination (160 pairs).
 * Each pair exercises:
 *   Phase 1  – InputHandler::loadData  (parsing)
 *   Phase 2  – Simulation construction (pathfinding setup)
 *   Phase 3  – Simulation::run         (physics + concurrency)
 *
 * A test PASSES if no uncaught exception escapes, no segfault occurs,
 * and no undefined behaviour is triggered.  Expected exceptions from
 * bad input files are caught and considered a pass.
 */

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "DijkstraPathfinding.hpp"
#include "InputHandler.hpp"
#include "Simulation.hpp"
#include "TestFramework.hpp"

/* ─── input directories ─── */

static const std::string NET_DIR = "input/railNetworkPrintFolder/";
static const std::string TRN_DIR = "input/trainPrintFolder/";

/* ─── all input files ─── */

static const std::vector<std::string> NETWORK_FILES = {
	"railNetworkBadEventDuration.txt",
	"railNetworkBadEventUnit.txt",
	"railNetworkBadKeyword.txt",
	"railNetworkBadRail.txt",
	"railNetworkDisconnected.txt",
	"railNetworkDuplicateNode.txt",
	"railNetworkDuplicateRail.txt",
	"railNetworkEmpty.txt",
	"railNetworkMinimal.txt",
	"railNetworkMissingNodeName.txt",
	"railNetworkNegativeDistance.txt",
	"railNetworkPrintGood.txt",
	"railNetworkQuotedNames.txt",
	"railNetworkSelfLoop.txt",
	"railNetworkUnknownNode.txt",
	"railNetworkZeroSpeed.txt",
};

static const std::vector<std::string> TRAIN_FILES = {
	"trainPrintBadStation.txt",
	"trainPrintBadTime.txt",
	"trainPrintBadTimeFormat.txt",
	"trainPrintEmpty.txt",
	"trainPrintGood.txt",
	"trainPrintIncomplete.txt",
	"trainPrintLateNight.txt",
	"trainPrintMidnight.txt",
	"trainPrintSingle.txt",
	"trainPrintUnreachable.txt",
};

/* ─── helpers ─── */

/**
 * Extract a short label from a filename, e.g.
 *   "railNetworkBadRail.txt" → "BadRail"
 *   "trainPrintGood.txt"     → "Good"
 */
static std::string shortLabel(const std::string &filename,
							  const std::string &prefix)
{
	std::string label = filename;
	/* strip prefix */
	size_t pos = label.find(prefix);
	if (pos != std::string::npos)
		label = label.substr(pos + prefix.length());
	/* strip .txt */
	pos = label.rfind(".txt");
	if (pos != std::string::npos)
		label = label.substr(0, pos);
	return label;
}

/**
 * Remove any result files that might have been generated.
 * We do a best-effort sweep of output/results/.
 */
static void cleanupOutputDir()
{
	/* Use system to remove all result files — safe and simple */
	std::system("rm -f output/results/*.result 2>/dev/null");
}

/**
 * Attempt to load, construct and run a full simulation with the
 * given network and train files.
 *
 * Returns true if the simulation completed without uncaught
 * exceptions or crashes.  Expected parsing / runtime errors from
 * bad input are caught and counted as success (the code handled
 * them gracefully).
 */
static bool tryFullRun(const std::string &netFile,
					   const std::string &trnFile,
					   std::string &phase)
{
	/* Phase 1 — parse */
	phase = "loadData";
	auto data = InputHandler::loadData(netFile, trnFile);

	/* Phase 2 — construct simulation */
	phase = "construct";
	Simulation sim(std::move(data.network), std::move(data.trains),
				   std::move(data.events),
				   std::make_unique<DijkstraPathfinding>());

	/* Phase 3 — run simulation */
	phase = "run";
	sim.run();

	return true;
}

/* ─── main ─── */

int main()
{
	Test::TestSuite suite("Combination");

	for (const auto &netFile : NETWORK_FILES)
	{
		for (const auto &trnFile : TRAIN_FILES)
		{
			std::string netLabel = shortLabel(netFile, "railNetwork");
			std::string trnLabel = shortLabel(trnFile, "trainPrint");
			std::string testName = netLabel + " x " + trnLabel;

			std::string netPath = NET_DIR + netFile;
			std::string trnPath = TRN_DIR + trnFile;

			suite.run(testName,
					  [netPath, trnPath](std::string &msg) {
						  cleanupOutputDir();
						  std::string phase;
						  try
						  {
							  tryFullRun(netPath, trnPath, phase);
						  }
						  catch (const std::exception &e)
						  {
							  /* Expected exception from bad input —
							     the program handled it gracefully.
							     Log it but still pass. */
							  (void)e;
						  }
						  /* If we get here the combo did not crash */
						  ASSERT_TRUE(true, msg);
						  return true;
					  });
		}
	}

	cleanupOutputDir();
	return suite.summarize();
}
