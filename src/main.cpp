/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 04:16:37 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sys/stat.h>
#include <vector>

#include "DijkstraPathfinding.hpp"
#include "GraphExporter.hpp"
#include "IPathfinding.hpp"
#include "InputHandler.hpp"
#include "Simulation.hpp"
#include "TerminalAnimDisplay.hpp"

static void printHelp()
{
	std::cout
		<< "Usage: ./Train <network_file> <train_file> [options]\n\n"
		<< "Options:\n"
		<< "  --time              Optimise route by travel time "
		<< "instead of distance\n"
		<< "  --graph <file.dot>  Export network + paths as "
		<< "Graphviz DOT file (default: output/graphs/)\n"
		<< "  --animate           Show live terminal animation of "
		<< "the simulation\n"
		<< "  --runs N            Run simulation N times and report "
		<< "average travel times\n\n"
		<< "=== Input files ===\n"
		<< "  Both input files use the .txt extension and must be\n"
		<< "  placed in the input/ directory:\n"
		<< "    input/railNetworkPrintFolder/<network>.txt\n"
		<< "    input/trainPrintFolder/<trains>.txt\n\n"
		<< "  Example:\n"
		<< "    ./Train input/railNetworkPrintFolder/"
		<< "railNetworkPrintGood.txt \\\n"
		<< "           input/trainPrintFolder/"
		<< "trainPrintGood.txt\n\n"
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
		<< "  The program generates one .result file per train in\n"
		<< "    output/results/TrainName_DepartureTime.result\n";
}

static std::string fmtTimeStat(double seconds)
{
	int total = static_cast<int>(seconds);
	int h = total / 3600;
	int m = (total % 3600) / 60;
	int s = total % 60;
	char buf[32];
	std::snprintf(buf, sizeof(buf), "%02dh%02dm%02ds", h, m, s);
	return buf;
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
	if (argc < 3 || argc > 10)
	{
		std::cerr << "Usage: " << argv[0]
				  << " <network_file> <train_file> [--time] "
				     "[--graph file.dot] [--animate] [--runs N]"
				  << std::endl;
		std::cerr << "Use --help for detailed format information."
				  << std::endl;
		return EXIT_FAILURE;
	}

	/* Validate .txt extension on input files */
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

	/* Parse optional flags */
	PathWeightMode weightMode = PathWeightMode::Distance;
	std::string graphFile;
	bool animate = false;
	int numRuns = 1;
	for (int i = 3; i < argc; i++)
	{
		if (std::strcmp(argv[i], "--time") == 0)
			weightMode = PathWeightMode::Time;
		else if (std::strcmp(argv[i], "--animate") == 0)
			animate = true;
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

	try
	{
		/* ---- Multi-run statistics collection ---- */
		/* Key = trainName, Value = vector of actualTimes */
		std::map<std::string, std::vector<double>> statActual;
		std::map<std::string, std::vector<double>> statDelay;
		std::map<std::string, double> statEstimated;

		for (int run = 0; run < numRuns; run++)
		{
			auto data = InputHandler::loadData(argv[1], argv[2]);
			auto pathfinder = std::make_unique<DijkstraPathfinding>();

			Simulation sim(std::move(data.network),
						   std::move(data.trains),
						   std::move(data.events),
						   std::move(pathfinder), weightMode);

			/* Suppress stdout on repeated runs (keep first run
			   verbose, or if --animate is on suppress all text) */
			if (numRuns > 1 && run > 0)
				sim.setQuiet(true);
			if (animate && numRuns == 1)
				sim.setQuiet(true);

			/* Attach animation display on the first (or only) run */
			TerminalAnimDisplay animDisplay;
			if (animate && run == 0)
			{
				sim.setAnimCallback(
					[&animDisplay](double t,
								   const std::vector<TrainState> &s) {
						animDisplay.render(t, s);
					});
			}

			sim.run();

			if (animate && run == 0)
				animDisplay.cleanup();

			/* Collect results */
			for (const auto &r : sim.getResults())
			{
				statActual[r.name].push_back(r.actualTime);
				statDelay[r.name].push_back(r.totalDelay);
				statEstimated[r.name] = r.estimatedTime;
			}

			/* Export graph only on first run */
			if (run == 0 && !graphFile.empty())
			{
				mkdir("output", 0755);
				mkdir("output/graphs", 0755);
				if (graphFile.find('/') == std::string::npos)
					graphFile = "output/graphs/" + graphFile;
				GraphExporter::exportDot(graphFile, sim.getNetwork(),
										 sim.getTrains());
			}
		}

		/* ---- Print multi-run statistics ---- */
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
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
