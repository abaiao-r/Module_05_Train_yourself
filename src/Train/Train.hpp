/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Train.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/22 13:05:50 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	int _id;
	std::string _name;
	double _weight;             // metric tons
	double _frictionCoefficient;
	double _maxAccelForce;      // kilonewtons
	double _maxBrakeForce;      // kilonewtons
	std::string _departureStation;
	std::string _arrivalStation;
	double _departureTime;      // seconds from midnight
	double _stopDuration;       // seconds at each station

	TrainStatus _status;
	std::vector<std::shared_ptr<Node>> _path;
	size_t _pathIndex;
	double _currentTime;
	double _totalDelay;
	double _currentSpeed;       // m/s
	double _posOnSegment;       // metres on current segment

  public:
	Train(int id, const std::string &name, double weight, double friction,
		  double accelForce, double brakeForce,
		  const std::string &departure, const std::string &arrival,
		  double departureTime, double stopDuration);
	Train(const Train &src);
	Train &operator=(const Train &src);
	~Train();

	void setPath(const std::vector<std::shared_ptr<Node>> &path);
	void advanceToNextNode(double travelTime);
	void applyDelay(double seconds);
	void setStatus(TrainStatus status);
	void setCurrentSpeed(double speed);
	void setPosOnSegment(double pos);
	void setPathIndex(size_t idx);
	void setCurrentTime(double t);

	int getId() const;
	const std::string &getName() const;
	double getWeight() const;
	double getFrictionCoefficient() const;
	double getMaxAccelForce() const;
	double getMaxBrakeForce() const;
	const std::string &getDepartureStation() const;
	const std::string &getArrivalStation() const;
	double getDepartureTime() const;
	double getStopDuration() const;
	TrainStatus getStatus() const;
	const std::vector<std::shared_ptr<Node>> &getPath() const;
	size_t getPathIndex() const;
	double getCurrentTime() const;
	double getTotalDelay() const;
	double getCurrentSpeed() const;
	double getPosOnSegment() const;
	std::string getCurrentNodeName() const;
	bool hasArrived() const;

	double getAccelRate() const;
	double getDecelRate() const;

	static std::string statusToString(TrainStatus status);
};

#endif
