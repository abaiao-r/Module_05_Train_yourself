/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EndToEndTest.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:00:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 15:19:32 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * End-to-End tests — runs the compiled ./bin/Train binary as a subprocess
 * and validates exit codes, stdout/stderr content, and generated output files.
 */

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include "TestFramework.hpp"

static const std::string BIN = "./bin/Train";
static const std::string NET =
	"input/railNetworkPrintFolder/railNetworkPrintGood.txt";
static const std::string TRN = "input/trainPrintFolder/trainPrintGood.txt";

/* ---- Helpers ---- */

/**
 * Run a shell command, capture combined stdout+stderr, return exit code.
 */
struct CmdResult
{
	int exitCode;
	std::string output;
};

static CmdResult runCmd(const std::string &cmd)
{
	CmdResult result;
	result.output.clear();
	std::string fullCmd = cmd + " 2>&1";
	FILE *pipe = popen(fullCmd.c_str(), "r");
	if (!pipe)
	{
		result.exitCode = -1;
		return result;
	}
	std::array<char, 4096> buf;
	while (fgets(buf.data(), static_cast<int>(buf.size()), pipe))
		result.output += buf.data();
	int status = pclose(pipe);
#ifdef __APPLE__
	result.exitCode = WEXITSTATUS(status);
#else
	result.exitCode = WEXITSTATUS(status);
#endif
	return result;
}

static bool outputContains(const std::string &output, const std::string &needle)
{
	return output.find(needle) != std::string::npos;
}

static void cleanupE2E()
{
	std::filesystem::remove_all("output/results");
	std::filesystem::remove_all("output/graphs");
}

/* ======================================================================== */

int main()
{
	Test::TestSuite suite("End-to-End");

	/* ------------------------------------------------------------------ */
	/*  CLI argument validation                                           */
	/* ------------------------------------------------------------------ */

	suite.run("no arguments -> exit 1 + usage message",
			  [](std::string &msg) {
				  auto r = runCmd(BIN);
				  ASSERT_EQ(1, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "Usage:"), msg);
				  return true;
			  });

	suite.run("--help -> exit 0 + help text",
			  [](std::string &msg) {
				  auto r = runCmd(BIN + " --help");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "Options:"), msg);
				  ASSERT_TRUE(outputContains(r.output, "--time"), msg);
				  ASSERT_TRUE(outputContains(r.output, "--graph"), msg);
				  ASSERT_TRUE(outputContains(r.output, "--animate"), msg);
				  ASSERT_TRUE(outputContains(r.output, "--runs"), msg);
				  return true;
			  });

	suite.run("unknown option -> exit 1",
			  [](std::string &msg) {
				  auto r = runCmd(BIN + " " + NET + " " + TRN + " --badopt");
				  ASSERT_EQ(1, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "Unknown option"),
							  msg);
				  return true;
			  });

	suite.run("missing network file -> exit 1",
			  [](std::string &msg) {
				  auto r = runCmd(BIN + " nonexistent.txt " + TRN);
				  ASSERT_EQ(1, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "Error"), msg);
				  return true;
			  });

	suite.run("non-.txt network file -> exit 1",
			  [](std::string &msg) {
				  auto r = runCmd(BIN + " README.md " + TRN);
				  ASSERT_EQ(1, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, ".txt"), msg);
				  return true;
			  });

	suite.run("--runs without number -> exit 1",
			  [](std::string &msg) {
				  auto r = runCmd(BIN + " " + NET + " " + TRN + " --runs");
				  ASSERT_EQ(1, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "requires"), msg);
				  return true;
			  });

	suite.run("--graph without filename -> exit 1",
			  [](std::string &msg) {
				  auto r = runCmd(BIN + " " + NET + " " + TRN + " --graph");
				  ASSERT_EQ(1, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "requires"), msg);
				  return true;
			  });

	/* ------------------------------------------------------------------ */
	/*  Default mode — full pipeline                                      */
	/* ------------------------------------------------------------------ */

	suite.run("default mode -> exit 0",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(BIN + " " + NET + " " + TRN);
				  ASSERT_EQ(0, r.exitCode, msg);
				  return true;
			  });

	suite.run("default mode -> prints Rail Network header",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(BIN + " " + NET + " " + TRN);
				  ASSERT_TRUE(outputContains(r.output, "Rail Network"), msg);
				  return true;
			  });

	suite.run("default mode -> prints Results section",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(BIN + " " + NET + " " + TRN);
				  ASSERT_TRUE(outputContains(r.output, "Results"), msg);
				  ASSERT_TRUE(outputContains(r.output, "Arrived"), msg);
				  return true;
			  });

	suite.run("default mode -> creates .result files",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(BIN + " " + NET + " " + TRN);
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(
					  std::filesystem::exists(
						  "output/results/TrainAB_14h10.result"),
					  msg);
				  ASSERT_TRUE(
					  std::filesystem::exists(
						  "output/results/TrainAC_14h20.result"),
					  msg);
				  ASSERT_TRUE(
					  std::filesystem::exists(
						  "output/results/TrainBA_14h24.result"),
					  msg);
				  return true;
			  });

	suite.run("default mode -> .result file has correct format",
			  [](std::string &msg) {
				  /* Relies on previous test having created the file */
				  std::ifstream f("output/results/TrainAB_14h10.result");
				  ASSERT_TRUE(f.is_open(), msg);
				  std::string line;
				  std::getline(f, line);
				  ASSERT_TRUE(outputContains(line, "Train : TrainAB"), msg);
				  std::getline(f, line);
				  ASSERT_TRUE(outputContains(line, "Final travel time"),
							  msg);
				  /* Skip blank line, then check first step line */
				  std::getline(f, line); // blank
				  std::getline(f, line); // first step
				  ASSERT_TRUE(outputContains(line, "["), msg);
				  ASSERT_TRUE(outputContains(line, "km]"), msg);
				  return true;
			  });

	suite.run("default mode -> .result file ends with actual travel time",
			  [](std::string &msg) {
				  std::ifstream f("output/results/TrainAB_14h10.result");
				  ASSERT_TRUE(f.is_open(), msg);
				  std::string lastLine;
				  std::string line;
				  while (std::getline(f, line))
				  {
					  if (!line.empty())
						  lastLine = line;
				  }
				  ASSERT_TRUE(
					  outputContains(lastLine, "Actual travel time"), msg);
				  return true;
			  });

	/* ------------------------------------------------------------------ */
	/*  --time mode                                                       */
	/* ------------------------------------------------------------------ */

	suite.run("--time mode -> exit 0 + trains arrive",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(BIN + " " + NET + " " + TRN + " --time");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "Arrived"), msg);
				  return true;
			  });

	/* ------------------------------------------------------------------ */
	/*  --graph mode                                                      */
	/* ------------------------------------------------------------------ */

	suite.run("--graph -> exit 0 + creates .dot file",
			  [](std::string &msg) {
				  cleanupE2E();
				  std::filesystem::create_directories("output/graphs");
				  auto r = runCmd(BIN + " " + NET + " " + TRN
								  + " --graph output/graphs/e2e_test.dot");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "Graph exported"),
							  msg);
				  ASSERT_TRUE(
					  std::filesystem::exists(
						  "output/graphs/e2e_test.dot"),
					  msg);
				  /* Verify DOT content */
				  std::ifstream f("output/graphs/e2e_test.dot");
				  std::string firstLine;
				  std::getline(f, firstLine);
				  ASSERT_TRUE(
					  outputContains(firstLine, "graph RailNetwork"), msg);
				  return true;
			  });

	suite.run("--graph -> auto-placed in output/graphs/ when no path",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(BIN + " " + NET + " " + TRN
								  + " --graph e2e_auto.dot");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(
					  std::filesystem::exists(
						  "output/graphs/e2e_auto.dot"),
					  msg);
				  /* Cleanup */
				  std::filesystem::remove("output/graphs/e2e_auto.dot");
				  std::filesystem::remove("output/graphs/e2e_auto.png");
				  std::filesystem::remove("output/graphs/e2e_auto.svg");
				  return true;
			  });

	/* ------------------------------------------------------------------ */
	/*  --runs mode                                                       */
	/* ------------------------------------------------------------------ */

	suite.run("--runs 2 -> exit 0 + multi-run statistics",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(BIN + " " + NET + " " + TRN + " --runs 2");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(
					  outputContains(r.output, "Multi-Run Statistics"),
					  msg);
				  ASSERT_TRUE(outputContains(r.output, "Avg Actual"), msg);
				  ASSERT_TRUE(outputContains(r.output, "Avg Delay"), msg);
				  return true;
			  });

	suite.run("--runs 1 -> exit 0 + no multi-run header",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(BIN + " " + NET + " " + TRN + " --runs 1");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_FALSE(
					  outputContains(r.output, "Multi-Run Statistics"),
					  msg);
				  return true;
			  });

	/* ------------------------------------------------------------------ */
	/*  Combined flags                                                    */
	/* ------------------------------------------------------------------ */

	suite.run("--time --graph combined -> exit 0",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(BIN + " " + NET + " " + TRN
								  + " --time --graph output/graphs/e2e_combo.dot");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(
					  std::filesystem::exists(
						  "output/graphs/e2e_combo.dot"),
					  msg);
				  ASSERT_TRUE(outputContains(r.output, "Arrived"), msg);
				  return true;
			  });

	suite.run("--time --runs 2 combined -> exit 0 + stats",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(BIN + " " + NET + " " + TRN
								  + " --time --runs 2");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(
					  outputContains(r.output, "Multi-Run Statistics"),
					  msg);
				  return true;
			  });

	/* ------------------------------------------------------------------ */
	/*  Error input files                                                 */
	/* ------------------------------------------------------------------ */

	suite.run("bad station in train file -> exit 1",
			  [](std::string &msg) {
				  auto r = runCmd(
					  BIN + " " + NET
					  + " input/trainPrintFolder/trainPrintBadStation.txt");
				  ASSERT_EQ(1, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "Error"), msg);
				  return true;
			  });

	suite.run("empty train file -> exit 0 (no trains to simulate)",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(
					  BIN + " " + NET
					  + " input/trainPrintFolder/trainPrintEmpty.txt");
				  ASSERT_EQ(0, r.exitCode, msg);
				  return true;
			  });

	suite.run("disconnected network + unreachable train -> warns no path",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(
					  BIN
					  + " input/railNetworkPrintFolder/"
					    "railNetworkDisconnected.txt "
					  + TRN);
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "No path found"),
							  msg);
				  return true;
			  });

	/* ------------------------------------------------------------------ */
	/*  Multiple networks (ensure different inputs work)                  */
	/* ------------------------------------------------------------------ */

	suite.run("France network + France trains -> exit 0",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(
					  BIN
					  + " input/railNetworkPrintFolder/"
					    "railNetworkFrance.txt"
					  + " input/trainPrintFolder/trainPrintFrance.txt");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "Arrived"), msg);
				  return true;
			  });

	suite.run("Europe network + Europe trains -> exit 0",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(
					  BIN
					  + " input/railNetworkPrintFolder/"
					    "railNetworkEurope.txt"
					  + " input/trainPrintFolder/trainPrintEurope.txt");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "Arrived"), msg);
				  return true;
			  });

	suite.run("Portugal network + Portugal trains -> exit 0",
			  [](std::string &msg) {
				  cleanupE2E();
				  auto r = runCmd(
					  BIN
					  + " input/railNetworkPrintFolder/"
					    "railNetworkPortugal.txt"
					  + " input/trainPrintFolder/trainPrintPortugal.txt");
				  ASSERT_EQ(0, r.exitCode, msg);
				  ASSERT_TRUE(outputContains(r.output, "Arrived"), msg);
				  return true;
			  });

	cleanupE2E();
	return suite.summarize();
}
