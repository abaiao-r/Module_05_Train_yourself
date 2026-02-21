#include "RailNetwork.hpp"

#include <algorithm>

/* ---- Canonical form ---- */
RailNetwork::RailNetwork() {}

RailNetwork::RailNetwork(const RailNetwork &src)
	: _nodes(src._nodes), _adjacencyList(src._adjacencyList)
{
}

RailNetwork &RailNetwork::operator=(const RailNetwork &src)
{
	if (this != &src)
	{
		_nodes = src._nodes;
		_adjacencyList = src._adjacencyList;
	}
	return *this;
}

RailNetwork::RailNetwork(RailNetwork &&src) noexcept
	: _nodes(std::move(src._nodes)),
	  _adjacencyList(std::move(src._adjacencyList))
{
}

RailNetwork &RailNetwork::operator=(RailNetwork &&src) noexcept
{
	if (this != &src)
	{
		_nodes = std::move(src._nodes);
		_adjacencyList = std::move(src._adjacencyList);
	}
	return *this;
}

RailNetwork::~RailNetwork() {}

/* ---- Graph operations ---- */
void RailNetwork::addNode(const std::string &name)
{
	if (_nodes.count(name))
		throw DuplicateNodeException(name);
	_nodes[name] = std::make_shared<Node>(name);
	_adjacencyList[name] = {};
}

void RailNetwork::addConnection(const std::string &from,
								const std::string &to, double distance,
								double speedLimit)
{
	if (!_nodes.count(from))
		throw NodeNotFoundException(from);
	if (!_nodes.count(to))
		throw NodeNotFoundException(to);
	if (from == to)
		throw std::invalid_argument("Cannot connect node to itself: " + from);
	if (distance <= 0.0)
		throw std::invalid_argument("Distance must be positive");
	if (speedLimit <= 0.0)
		throw std::invalid_argument("Speed limit must be positive");

	for (const auto &edge : _adjacencyList[from])
	{
		if (edge.destination->getName() == to)
			throw DuplicateConnectionException(from, to);
	}

	_adjacencyList[from].push_back({_nodes[to], distance, speedLimit});
	_adjacencyList[to].push_back({_nodes[from], distance, speedLimit});
}

std::shared_ptr<Node> RailNetwork::findNode(const std::string &name) const
{
	auto it = _nodes.find(name);
	if (it == _nodes.end())
		throw NodeNotFoundException(name);
	return it->second;
}

const std::vector<Edge> &RailNetwork::getNeighbours(
	const std::string &nodeName) const
{
	auto it = _adjacencyList.find(nodeName);
	if (it == _adjacencyList.end())
		throw NodeNotFoundException(nodeName);
	return it->second;
}

std::vector<std::string> RailNetwork::getNodeNames() const
{
	std::vector<std::string> names;
	names.reserve(_nodes.size());
	for (const auto &pair : _nodes)
		names.push_back(pair.first);
	std::sort(names.begin(), names.end());
	return names;
}

size_t RailNetwork::nodeCount() const { return _nodes.size(); }

/* ---- Exception implementations ---- */
RailNetwork::NodeNotFoundException::NodeNotFoundException(
	const std::string &name)
	: std::runtime_error("Node not found: " + name)
{
}

RailNetwork::DuplicateNodeException::DuplicateNodeException(
	const std::string &name)
	: std::runtime_error("Node already exists: " + name)
{
}

RailNetwork::DuplicateConnectionException::DuplicateConnectionException(
	const std::string &from, const std::string &to)
	: std::runtime_error("Connection already exists: " + from + " <-> " + to)
{
}
