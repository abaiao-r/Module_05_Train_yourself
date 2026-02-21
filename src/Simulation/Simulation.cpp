#include "Simulation.hpp"

#include <algorithm>
#include <iostream>

#include "Node.hpp"

/* ---- Constructor / Destructor ---- */
Simulation::Simulation(RailNetwork network,
					   std::vector<std::unique_ptr<Train>> trains,
					   std::vector<Event> events,
					   std::unique_ptr<IPathfinding> pathfinder)
	: _network(std::move(network)), _trains(std::move(trains)),
	  _events(std::move(events)), _pathfinder(std::move(pathfinder)),
	  _rng(std::random_device{}())
{
}

Simulation::~Simulation() {}

/* ---- Public ---- */
void Simulation::run()
{
	_output.printNetwork(_network);
	computePaths();

	std::cout << "=== Simulation ===" << std::endl;
	std::sort(_trains.begin(), _trains.end(),
			  [](const auto &a, const auto &b) {
				  return a->getDepartureTime() < b->getDepartureTime();
			  });

	for (auto &train : _trains)
		simulateTrain(*train);

	_output.printResult(_trains);
}

/* ---- Private ---- */
void Simulation::computePaths()
{
	std::cout << "=== Paths ===" << std::endl;
	for (auto &train : _trains)
	{
		auto path = _pathfinder->findPath(train->getDepartureStation(),
										  train->getArrivalStation(),
										  _network);
		if (path.empty())
		{
			std::cerr << "No path found for " << train->getName() << " ("
					  << train->getDepartureStation() << " -> "
					  << train->getArrivalStation() << ")" << std::endl;
			continue;
		}
		train->setPath(path);
		_output.printTrainPath(*train);
	}
	std::cout << std::endl;
}

void Simulation::simulateTrain(Train &train)
{
	const auto &path = train.getPath();
	if (path.size() < 2)
		return;

	train.setStatus(TrainStatus::Running);
	_output.printDeparture(train);

	for (size_t i = 0; i < path.size() - 1; i++)
	{
		const std::string &from = path[i]->getName();
		const std::string &to = path[i + 1]->getName();
		double travelTime = getEdgeTravelTime(from, to);
		train.advanceToNextNode(travelTime);
		_output.printArrival(train, to, train.getCurrentTime());

		auto nodeEvents = getEventsAtNode(to);
		for (const auto *event : nodeEvents)
		{
			if (event->tryTrigger(_rng))
			{
				train.applyDelay(event->getDuration());
				_output.printEvent(*event, train);
			}
		}
	}
	std::cout << std::endl;
}

double Simulation::getEdgeTravelTime(const std::string &from,
									 const std::string &to) const
{
	const auto &edges = _network.getNeighbours(from);
	for (const auto &edge : edges)
	{
		if (edge.destination->getName() == to)
			return (edge.distance / edge.speedLimit) * 3600.0;
	}
	return 0.0;
}

std::vector<const Event *> Simulation::getEventsAtNode(
	const std::string &nodeName) const
{
	std::vector<const Event *> result;
	for (const auto &event : _events)
	{
		if (event.getNodeName() == nodeName)
			result.push_back(&event);
	}
	return result;
}
