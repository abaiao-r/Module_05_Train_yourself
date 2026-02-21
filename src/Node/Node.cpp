#include "Node.hpp"

Node::Node(const std::string &name) : _name(name)
{
	if (name.empty())
		throw InvalidNameException("Node name cannot be empty");
}

Node::Node(const Node &src) : _name(src._name) {}

Node &Node::operator=(const Node &src)
{
	if (this != &src)
		_name = src._name;
	return *this;
}

Node::~Node() {}

const std::string &Node::getName() const { return _name; }

Node::InvalidNameException::InvalidNameException(const std::string &msg)
	: std::invalid_argument(msg)
{
}
