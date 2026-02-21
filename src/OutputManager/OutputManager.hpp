#ifndef OUTPUTMANAGER_HPP
#define OUTPUTMANAGER_HPP

#include <memory>
#include <string>
#include <vector>

#include "Event.hpp"
#include "RailNetwork.hpp"
#include "Train.hpp"

class OutputManager
{
  public:
	OutputManager();
	OutputManager(const OutputManager &);
	OutputManager &operator=(const OutputManager &);
	~OutputManager();

	void printNetwork(const RailNetwork &network) const;
	void printTrainPath(const Train &train) const;
	void printDeparture(const Train &train) const;
	void printArrival(const Train &train, const std::string &nodeName,
					  double time) const;
	void printEvent(const Event &event, const Train &train) const;
	void printResult(
		const std::vector<std::unique_ptr<Train>> &trains) const;

	static std::string formatTime(double seconds);
};

#endif
