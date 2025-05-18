#pragma once
#include <memory>

class Node;	 // forward declaration

struct Edge
{
		std::weak_ptr<Node> node;
		size_t				distance;
		size_t				speedLimit;
};
