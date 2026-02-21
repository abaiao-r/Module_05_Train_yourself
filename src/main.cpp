#include <cstdlib>
#include <iostream>
#include <memory>

#include "DijkstraPathfinding.hpp"
#include "InputHandler.hpp"
#include "Simulation.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0]
				  << " <network_file> <train_file>" << std::endl;
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
