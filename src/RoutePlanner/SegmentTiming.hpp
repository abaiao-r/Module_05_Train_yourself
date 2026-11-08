#ifndef SEGMENTTIMING_HPP
#define SEGMENTTIMING_HPP

#include "Edge.hpp"

/** Constant-speed travel time of a rail in seconds (lane < 0 = edge
	speed). Planners use it to place reservations on a timeline. */
inline double freeFlowSeconds(const Edge &edge, int lane = -1)
{
	double kmh = lane < 0 ? edge.speedLimit : edge.lineSpeed(lane);
	return edge.distance / kmh * 3600.0;
}

#endif
