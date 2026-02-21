/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 09:57:55 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include "DijkstraPathfinding.hpp"
#include "InputHandler.hpp"
#include "Simulation.hpp"

static void printHelp()
{
	std::cout
		<< "Usage: ./Train <network_file> <train_file>\n\n"
		<< "=== Network file format ===\n"
		<< "  Node <name>\n"
		<< "    Declares a station or junction in the network.\n\n"
		<< "  Rail <from> <to> <distance_km> <speed_limit_kmh>\n"
		<< "    Creates a bidirectional track between two nodes.\n\n"
		<< "  Event <name> <probability> <duration> <node_name>\n"
		<< "    Adds a random event at a node. Duration units: m/h/d.\n"
		<< "    Quoted names are supported: \"Passenger's Discomfort\"\n\n"
		<< "=== Train file format ===\n"
		<< "  <name> <weight_t> <friction> <accel_kN> <brake_kN> "
		<< "<from> <to> <departure_time> <stop_duration>\n"
		<< "    Example: TrainAB 80 0.05 356.0 30.0 CityA CityB "
		<< "14h10 00h10\n\n"
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
		<< "=== Output ===\n"
		<< "  The program generates one .result file per train:\n"
		<< "    TrainName_DepartureTime.result\n";
}

int main(int argc, char **argv)
{
	if (argc == 2 && std::strcmp(argv[1], "--help") == 0)
	{
		printHelp();
		return EXIT_SUCCESS;
	}
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0]
				  << " <network_file> <train_file>" << std::endl;
		std::cerr << "Use --help for detailed format information."
				  << std::endl;
		return EXIT_FAILURE;
	}

	try
	{
		auto data = InputHandler::loadData(argv[1], argv[2]);
		auto pathfinder = std::make_unique<DijkstraPathfinding>();

		Simulation sim(std::move(data.network), std::move(data.trains),
					   std::move(data.events), std::move(pathfinder));
		sim.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
