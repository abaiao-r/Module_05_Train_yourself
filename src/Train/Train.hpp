#ifndef TRAIN_HPP
#define TRAIN_HPP

#include <memory>
#include <string>
#include <vector>

class Node;

enum class TrainStatus
{
	Waiting,
	Running,
	Arrived,
	Delayed
};

class Train
{
  private:
	std::string _name;
	double _maxAcceleration;
	double _maxBrakingForce;
	std::string _departureStation;
	std::string _arrivalStation;
	double _departureTime;

	TrainStatus _status;
	std::vector<std::shared_ptr<Node>> _path;
	size_t _pathIndex;
	double _currentTime;
	double _totalDelay;

  public:
	Train(const std::string &name, double acceleration, double braking,
		  const std::string &departure, const std::string &arrival,
		  double departureTime);
	Train(const Train &src);
	Train &operator=(const Train &src);
	~Train();

	void setPath(const std::vector<std::shared_ptr<Node>> &path);
	void advanceToNextNode(double travelTime);
	void applyDelay(double seconds);
	void setStatus(TrainStatus status);

	const std::string &getName() const;
	double getMaxAcceleration() const;
	double getMaxBrakingForce() const;
	const std::string &getDepartureStation() const;
	const std::string &getArrivalStation() const;
	double getDepartureTime() const;
	TrainStatus getStatus() const;
	const std::vector<std::shared_ptr<Node>> &getPath() const;
	size_t getPathIndex() const;
	double getCurrentTime() const;
	double getTotalDelay() const;
	std::string getCurrentNodeName() const;
	bool hasArrived() const;

	static std::string statusToString(TrainStatus status);
};

#endif
