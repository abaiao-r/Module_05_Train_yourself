#include "Event.hpp"

Event::Event(const std::string &name, double probability, double duration,
			 const std::string &nodeName)
	: _name(name), _probability(probability), _duration(duration),
	  _nodeName(nodeName)
{
}

Event::Event(const Event &src)
	: _name(src._name), _probability(src._probability),
	  _duration(src._duration), _nodeName(src._nodeName)
{
}

Event &Event::operator=(const Event &src)
{
	if (this != &src)
	{
		_name = src._name;
		_probability = src._probability;
		_duration = src._duration;
		_nodeName = src._nodeName;
	}
	return *this;
}

Event::~Event() {}

bool Event::tryTrigger(std::mt19937 &rng) const
{
	if (_probability <= 0.0)
		return false;
	std::uniform_real_distribution<double> dist(0.0, 1.0);
	return dist(rng) < _probability;
}

const std::string &Event::getName() const { return _name; }
double Event::getProbability() const { return _probability; }
double Event::getDuration() const { return _duration; }
const std::string &Event::getNodeName() const { return _nodeName; }
