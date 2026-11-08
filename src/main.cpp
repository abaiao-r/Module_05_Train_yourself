/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 19:17:55 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include "DijkstraPathfinding.hpp"
#include "PrioritizedPlanner.hpp"
#include "RefinedPlanner.hpp"
#include "CbsPlanner.hpp"
#include "GraphExporter.hpp"
#include "IPathfinding.hpp"
#include "InputHandler.hpp"
#include "Simulation.hpp"
#include "ParallelFor.hpp"
#include "TerminalAnimDisplay.hpp"
#include "TrainFactory.hpp"

static void printHelp()
{
	std::cout
		<< "Usage: ./train_yourself <network_file> <train_file> [options]\n\n"
		<< "Options:\n"
		<< "  --distance          Shortest distance (km)\n"
		<< "  --solo              Fastest way if this train were the "
		<< "only one on\n"
		<< "                      the track — optimises by travel time, "
		<< "ignoring\n"
		<< "                      every other train and every random "
		<< "event. This is\n"
		<< "                      the default.\n"
		<< "  --adaptive          The ultimate mode: fastest way "
		<< "accounting for real\n"
		<< "                      conditions — reacts to live traffic "
		<< "(dynamically\n"
		<< "                      reroutes around occupied segments) "
		<< "AND avoids routes\n"
		<< "                      prone to costly random events. Goal: "
		<< "every train\n"
		<< "                      arrives as fast as possible.\n"
		<< "                      (--distance, --solo and --adaptive "
		<< "are\n"
		<< "                      mutually exclusive — pick one)\n"
		<< "  --graph <file.dot>  Export network + paths as "
		<< "Graphviz DOT file\n"
		<< "                      Auto-renders PNG & SVG if graphviz is "
		<< "installed\n"
		<< "                      (default dir: output/graphs/)\n"
		<< "  --animate           Show live terminal animation of "
		<< "the simulation\n"
		<< "  --runs N            Run simulation N times and report "
		<< "average travel times\n"
		<< "  --planner <p>       Bonus: independent (default, every train picks "
		<< "its own\n"
		<< "                      route) or cooperative (the fleet is routed "
		<< "together,\n"
		<< "                      trains avoid each other; plans by travel "
		<< "time, so it\n"
		<< "                      excludes --distance and --adaptive) or cbs\n"
		<< "                      (conflict-based search on head-on rails)\n"
		<< "  --compare           Run all modes plus the cooperative and cbs planners "
		<< "(with\n"
		<< "                      --runs N each) and print them side by side\n"
		<< "  --lines             Bonus: rails may hold several lines "
		<< "(optional\n"
		<< "                      trailing count on Rail); opposite-"
		<< "direction trains\n"
		<< "                      never share a line\n"
		<< "  --help              Show this help message\n\n"
		<< "=== Build targets ===\n"
		<< "  make                Build the CLI binary (./bin/train_yourself)\n"
		<< "  make test           Run all test suites (unit + E2E)\n"
		<< "  make bonus          Build the Qt 6 GUI (./bin/TrainGUI)\n"
		<< "  make run-gui        Build and launch the GUI\n"
		<< "  make run-animate    Run with terminal animation\n"
		<< "  make run-multi      Run 1000 iterations with statistics\n\n"
		<< "=== Input files ===\n"
		<< "  Both input files use the .txt extension and must be\n"
		<< "  placed in the input/ directory:\n"
		<< "    input/railNetworkPrintFolder/<network>.txt\n"
		<< "    input/trainPrintFolder/<trains>.txt\n\n"
		<< "  Example:\n"
		<< "    ./train_yourself input/railNetworkPrintFolder/"
		<< "railNetworkNormal3Events.txt \\\n"
		<< "           input/trainPrintFolder/"
		<< "trainPrintNormal3Events.txt\n\n"
		<< "=== Network file format (.txt) ===\n"
		<< "  Node <name>\n"
		<< "    Declares a station or junction in the network.\n\n"
		<< "  Rail <from> <to> <distance_km> <speed_limit_kmh>\n"
		<< "    Creates a bidirectional track between two nodes.\n\n"
		<< "  Event <name> <probability> <duration> <node_name> [node_name2]\n"
		<< "    Adds a random event at a node or on a rail segment.\n"
		<< "    One node = city event; two nodes = rail segment event.\n"
		<< "    Duration units: m/h/d.\n"
		<< "    Quoted names are supported: \"Passenger's Discomfort\"\n\n"
		<< "  Example network file:\n"
		<< "    Node CityA\n"
		<< "    Node CityB\n"
		<< "    Node CityC\n"
		<< "    Rail CityA CityB 100 120\n"
		<< "    Rail CityB CityC 80 100\n"
		<< "    Event Delay 0.3 30m CityB\n\n"
		<< "=== Train file format (.txt) ===\n"
		<< "  <name> <weight_t> <friction> <accel_kN> <brake_kN> "
		<< "<from> <to> <departure_time> <stop_duration>\n\n"
		<< "  Fields:\n"
		<< "    name            - Train identifier\n"
		<< "    weight_t        - Weight in metric tons\n"
		<< "    friction        - Coefficient of friction\n"
		<< "    accel_kN        - Max acceleration force (kilonewtons)\n"
		<< "    brake_kN        - Max braking force (kilonewtons)\n"
		<< "    from            - Departure station\n"
		<< "    to              - Arrival station\n"
		<< "    departure_time  - Departure time (e.g. 14h10)\n"
		<< "    stop_duration   - Duration of stop at each station "
		<< "(e.g. 00h10)\n\n"
		<< "  Example train file:\n"
		<< "    TrainAB 80 0.05 356.0 30.0 CityA CityB 14h10 00h10\n"
		<< "    TrainBC 60 0.04 300.0 25.0 CityB CityC 15h00 00h05\n\n"
		<< "=== Output ===\n"
		<< "  Per-train result files:\n"
		<< "    output/results/TrainName_DepartureTime.result\n\n"
		<< "  With --graph:\n"
		<< "    <file>.dot   Graphviz source\n"
		<< "    <file>.png   Rendered graph (if graphviz installed)\n"
		<< "    <file>.svg   Rendered graph (if graphviz installed)\n\n"
		<< "=== Bonus features ===\n"
		<< "  --animate       Live terminal animation with progress "
		<< "bars\n"
		<< "  --runs N        Monte-Carlo statistics across N "
		<< "simulation runs\n"
		<< "  --lines         Multi-line rails with head-on collision "
		<< "avoidance\n"
		<< "  make bonus      Qt 6 GUI with CRUD, drag-and-drop, "
		<< "dark theme,\n"
		<< "                  presets, import/export, and live "
		<< "simulation\n";
}

static std::string fmtTimeStat(double seconds)
{
	int total = static_cast<int>(std::lround(seconds));
	int h = total / 3600;
	int m = (total % 3600) / 60;
	int s = total % 60;
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << h << "h"
		<< std::setw(2) << m << "m"
		<< std::setw(2) << s << "s";
	return oss.str();
}

static bool hasTxtExtension(const std::string &path)
{
	return path.size() >= 4
		   && path.compare(path.size() - 4, 4, ".txt") == 0;
}

int main(int argc, char **argv)
{
	/* --help may appear at any position */
	for (int i = 1; i < argc; i++)
	{
		if (std::strcmp(argv[i], "--help") == 0)
		{
			printHelp();
			return EXIT_SUCCESS;
		}
	}
	if (argc < 3 || argc > 14)
	{
		std::cerr << "Usage: " << argv[0]
				  << " <network_file> <train_file> [--distance|--solo|--adaptive] "
				     "[--planner independent|cooperative|cbs] [--graph file.dot] "
				     "[--animate] [--runs N] [--compare] [--lines]"
				  << std::endl;
		std::cerr << "Use --help for detailed format information."
				  << std::endl;
		return EXIT_FAILURE;
	}

	if (!hasTxtExtension(argv[1]))
	{
		std::cerr << "Error: Network file must have .txt extension: "
				  << argv[1] << std::endl;
		return EXIT_FAILURE;
	}
	if (!hasTxtExtension(argv[2]))
	{
		std::cerr << "Error: Train file must have .txt extension: "
				  << argv[2] << std::endl;
		return EXIT_FAILURE;
	}

	PathWeightMode weightMode = PathWeightMode::Solo;
	bool weightModeSet = false;
	std::string graphFile;
	bool animate = false;
	bool compare = false;
	bool cooperative = false;
	bool cbs = false;
	LinesOptions lines;
	int numRuns = 1;
	for (int i = 3; i < argc; i++)
	{
		if (std::strcmp(argv[i], "--distance") == 0)
		{
			if (weightModeSet)
			{
				std::cerr << "Error: --distance, --solo and --adaptive are "
							  "mutually exclusive" << std::endl;
				return EXIT_FAILURE;
			}
			weightMode = PathWeightMode::Distance;
			weightModeSet = true;
		}
		else if (std::strcmp(argv[i], "--solo") == 0)
		{
			if (weightModeSet)
			{
				std::cerr << "Error: --distance, --solo and --adaptive are "
							  "mutually exclusive" << std::endl;
				return EXIT_FAILURE;
			}
			weightMode = PathWeightMode::Solo;
			weightModeSet = true;
		}
		else if (std::strcmp(argv[i], "--adaptive") == 0)
		{
			if (weightModeSet)
			{
				std::cerr << "Error: --distance, --solo and --adaptive are "
							  "mutually exclusive" << std::endl;
				return EXIT_FAILURE;
			}
			weightMode = PathWeightMode::Adaptive;
			weightModeSet = true;
		}
		else if (std::strcmp(argv[i], "--animate") == 0)
			animate = true;
		else if (std::strcmp(argv[i], "--lines") == 0)
			lines.enabled = true;
		else if (std::strcmp(argv[i], "--compare") == 0)
			compare = true;
		else if (std::strcmp(argv[i], "--planner") == 0)
		{
			if (i + 1 >= argc)
			{
				std::cerr << "--planner requires independent|cooperative|cbs"
						  << std::endl;
				return EXIT_FAILURE;
			}
			std::string p = argv[++i];
			if (p == "cooperative")
				cooperative = true;
			else if (p == "cbs")
				cooperative = cbs = true;
			else if (p != "independent")
			{
				std::cerr << "Error: unknown planner '" << p
						  << "' (expected independent|cooperative|cbs)"
						  << std::endl;
				return EXIT_FAILURE;
			}
		}
		else if (std::strcmp(argv[i], "--runs") == 0)
		{
			if (i + 1 >= argc)
			{
				std::cerr << "--runs requires a number" << std::endl;
				return EXIT_FAILURE;
			}
			numRuns = std::atoi(argv[++i]);
			if (numRuns < 1)
			{
				std::cerr << "--runs must be >= 1" << std::endl;
				return EXIT_FAILURE;
			}
		}
		else if (std::strcmp(argv[i], "--graph") == 0)
		{
			if (i + 1 >= argc)
			{
				std::cerr << "--graph requires a filename" << std::endl;
				return EXIT_FAILURE;
			}
			graphFile = argv[++i];
		}
		else
		{
			std::cerr << "Unknown option: " << argv[i] << std::endl;
			return EXIT_FAILURE;
		}
	}

	if (compare && weightModeSet)
	{
		std::cerr << "Error: --compare runs every mode; do not combine it "
					 "with --distance, --solo or --adaptive" << std::endl;
		return EXIT_FAILURE;
	}

	if (cooperative && compare)
	{
		std::cerr << "Error: --compare already includes the cooperative and cbs "
					 "planner; do not combine it with --planner"
				  << std::endl;
		return EXIT_FAILURE;
	}
	if (cooperative && !weightModeSet)
		weightMode = PathWeightMode::Adaptive;

	try
	{
		struct Stats
		{
			std::map<std::string, std::vector<double>> actual;
			std::map<std::string, std::vector<double>> delay;
			std::map<std::string, double> estimated;
		};

		/* One simulation run. Only the primary run 0 prints, animates, writes
		   the result files and exports the graph; every other run is quiet
		   and independent, so they can execute in parallel. */
		auto runOnce = [&](PathWeightMode mode, bool coop, bool useCbs,
						   bool primary, int run) {
			Stats st;
			const bool first = primary && run == 0;
			TrainFactory::resetIdCounter();
			auto data = InputHandler::loadData(argv[1], argv[2], lines);
			auto pathfinder = std::make_unique<DijkstraPathfinding>();

			Simulation sim(std::move(data.network),
						   std::move(data.trains),
						   std::move(data.events),
						   std::move(pathfinder), mode, data.lines);

			if (coop && useCbs)
				sim.setRoutePlanner(std::make_unique<CbsPlanner>());
			else if (coop)
				sim.setRoutePlanner(std::make_unique<RefinedPlanner>(
					std::make_unique<PrioritizedPlanner>(),
					std::make_unique<IndependentPlanner>()));
			if (!first)
			{
				sim.setQuiet(true);
				sim.setWriteResults(false);
			}
			if (first && animate && numRuns == 1)
				sim.setQuiet(true);

			/* Its destructor writes to cout, so only run 0 owns one. */
			std::unique_ptr<TerminalAnimDisplay> animPtr;
			if (first)
				animPtr = std::make_unique<TerminalAnimDisplay>();
			if (first && animate)
			{
				TerminalAnimDisplay &animDisplay = *animPtr;
				sim.setAnimCallback(
					[&animDisplay](double t,
								   const std::vector<TrainState> &s) {
						animDisplay.render(t, s);
					});
			}

			sim.run();

			if (first && animate)
				animPtr->cleanup();

			/* Same-named trains (different departures) get separate
			   rows: results keep load order, so the Nth duplicate is
			   stable. */
			std::map<std::string, int> seen;
			for (const auto &r : sim.getResults())
			{
				int n = seen[r.name]++;
				std::string key = n ? r.name + " #" + std::to_string(n + 1)
									: r.name;
				st.actual[key].push_back(r.actualTime);
				st.delay[key].push_back(r.totalDelay);
				st.estimated[key] = r.estimatedTime;
			}

			if (first && !graphFile.empty())
			{
				std::filesystem::create_directories("output/graphs");
				if (graphFile.find('/') == std::string::npos)
					graphFile = "output/graphs/" + graphFile;
				GraphExporter::exportDot(graphFile, sim.getNetwork(),
										 sim.getTrains());
			}
			return st;
		};

		/* Merging in run order keeps the statistics deterministic. */
		auto merge = [](Stats &dst, const Stats &src) {
			for (const auto &kv : src.actual)
				dst.actual[kv.first].insert(dst.actual[kv.first].end(),
											kv.second.begin(),
											kv.second.end());
			for (const auto &kv : src.delay)
				dst.delay[kv.first].insert(dst.delay[kv.first].end(),
										   kv.second.begin(),
										   kv.second.end());
			for (const auto &kv : src.estimated)
				dst.estimated[kv.first] = kv.second;
		};

		auto collect = [&](PathWeightMode mode, bool coop, bool useCbs,
						   bool primary, Stats &st) {
			std::vector<Stats> per(static_cast<size_t>(numRuns));
			int firstParallel = 0;
			if (primary)
			{
				per[0] = runOnce(mode, coop, useCbs, true, 0);
				firstParallel = 1;
			}
			parallelFor(numRuns - firstParallel, [&](int i) {
				int run = firstParallel + i;
				per[static_cast<size_t>(run)] =
					runOnce(mode, coop, useCbs, primary, run);
			});
			for (const auto &r : per)
				merge(st, r);
		};

		if (compare)
		{
			const int N = 5;
			const PathWeightMode modes[N] = {PathWeightMode::Distance,
											 PathWeightMode::Solo,
											 PathWeightMode::Adaptive,
											 PathWeightMode::Adaptive,
											 PathWeightMode::Adaptive};
			const char *names[N] = {"Distance", "Solo", "Adaptive",
									"Cooperative", "CBS"};
			Stats res[N];
			/* Every (mode, run) pair is independent: spread them all over the
			   cores at once. */
			std::vector<Stats> per(static_cast<size_t>(N) * numRuns);
			parallelFor(N * numRuns, [&](int k) {
				int m = k / numRuns;
				per[static_cast<size_t>(k)] =
					runOnce(modes[m], m >= 3, m == 4, false, k % numRuns);
			});
			for (int k = 0; k < N * numRuns; k++)
				merge(res[k / numRuns], per[static_cast<size_t>(k)]);

			auto avgOf = [](const std::vector<double> &v) {
				double t = 0;
				for (double x : v)
					t += x;
				return t / v.size();
			};
			auto printTable = [&](const char *title,
								  std::map<std::string, std::vector<double>>
									  Stats::*field) {
				std::cout << "=== " << title << " (" << numRuns
						  << (numRuns == 1 ? " run" : " runs")
						  << " each, * = best) ===" << std::endl;
				std::cout << std::string(90, '-') << std::endl;
				std::cout << std::left << std::setw(20) << "Train";
				for (int m = 0; m < N; m++)
					std::cout << std::setw(14) << names[m];
				std::cout << std::endl << std::string(90, '-') << std::endl;

				auto printRow = [&](const std::string &label,
									const double v[N]) {
					int best = 0;
					for (int m = 1; m < N; m++)
						if (v[m] < v[best] - 0.5)
							best = m;
					std::cout << std::left << std::setw(20) << label;
					for (int m = 0; m < N; m++)
					{
						bool star = v[m] <= v[best] + 0.5;
						std::cout << std::setw(14)
								  << (fmtTimeStat(v[m]) + (star ? " *" : ""));
					}
					std::cout << std::endl;
				};
				double total[N] = {0, 0, 0, 0, 0};
				for (const auto &pair : res[0].actual)
				{
					double v[N];
					for (int m = 0; m < N; m++)
					{
						auto &mp = res[m].*field;
						auto it = mp.find(pair.first);
						v[m] = it == mp.end() ? 0.0 : avgOf(it->second);
						total[m] += v[m];
					}
					printRow(pair.first, v);
				}
				std::cout << std::string(90, '-') << std::endl;
				printRow("TOTAL (sum)", total);
				std::cout << std::string(90, '-') << std::endl;
			};
			printTable("Mode comparison: average travel time", &Stats::actual);
			std::cout << std::endl;
			printTable("Mode comparison: average delay", &Stats::delay);
			return EXIT_SUCCESS;
		}

		Stats stats;
		collect(weightMode, cooperative, cbs, true, stats);
		auto &statActual = stats.actual;
		auto &statDelay = stats.delay;
		auto &statEstimated = stats.estimated;

		if (numRuns > 1)
		{
			std::cout << "\n=== Multi-Run Statistics (" << numRuns
					  << " runs) ===" << std::endl;
			std::cout << std::string(72, '-') << std::endl;
			std::cout << std::left << std::setw(14) << "Train"
					  << std::setw(14) << "Estimated"
					  << std::setw(14) << "Avg Actual"
					  << std::setw(14) << "Min Actual"
					  << std::setw(14) << "Max Actual"
					  << "Avg Delay" << std::endl;
			std::cout << std::string(72, '-') << std::endl;

			for (const auto &pair : statActual)
			{
				const std::string &name = pair.first;
				const auto &times = pair.second;
				const auto &delays = statDelay[name];

				double sum = 0, mn = times[0], mx = times[0];
				for (double t : times)
				{
					sum += t;
					if (t < mn) mn = t;
					if (t > mx) mx = t;
				}
				double avg = sum / times.size();

				double dsum = 0;
				for (double d : delays)
					dsum += d;
				double avgDelay = dsum / delays.size();

				std::cout << std::left
						  << std::setw(14) << name
						  << std::setw(14)
						  << fmtTimeStat(statEstimated[name])
						  << std::setw(14) << fmtTimeStat(avg)
						  << std::setw(14) << fmtTimeStat(mn)
						  << std::setw(14) << fmtTimeStat(mx)
						  << fmtTimeStat(avgDelay) << std::endl;
			}
			std::cout << std::string(72, '-') << std::endl;

			/* Sum over all trains; min/max use each run's own total. */
			double tEst = 0, tAvg = 0, tDelay = 0;
			std::vector<double> perRun(numRuns, 0.0);
			for (const auto &pair : statActual)
			{
				tEst += statEstimated[pair.first];
				for (size_t r = 0; r < pair.second.size(); r++)
				{
					perRun[r] += pair.second[r];
					tAvg += pair.second[r] / numRuns;
					tDelay += statDelay[pair.first][r] / numRuns;
				}
			}
			double tMin = perRun[0], tMax = perRun[0];
			for (double t : perRun)
			{
				if (t < tMin) tMin = t;
				if (t > tMax) tMax = t;
			}
			std::cout << std::left << std::setw(14) << "TOTAL (sum)"
					  << std::setw(14) << fmtTimeStat(tEst)
					  << std::setw(14) << fmtTimeStat(tAvg)
					  << std::setw(14) << fmtTimeStat(tMin)
					  << std::setw(14) << fmtTimeStat(tMax)
					  << fmtTimeStat(tDelay) << std::endl;
			std::cout << std::string(72, '-') << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
