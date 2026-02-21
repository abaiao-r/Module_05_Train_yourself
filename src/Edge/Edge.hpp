#ifndef EDGE_HPP
#define EDGE_HPP

#include <memory>

class Node;

struct Edge
{
	std::shared_ptr<Node> destination;
	double distance;    // km
	double speedLimit;  // km/h
};

#endif
