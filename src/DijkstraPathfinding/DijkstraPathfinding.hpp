/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraPathfinding.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIJKSTRAPATHFINDING_HPP
#define DIJKSTRAPATHFINDING_HPP

#include <memory>
#include <vector>

#include "IPathfindingAlgorithm.hpp"
#include "Node.hpp"

class DijkstraPathfinding : public IPathfindingAlgorithm
{
  public:
	DijkstraPathfinding();
	DijkstraPathfinding(const DijkstraPathfinding &src);
	DijkstraPathfinding &operator=(const DijkstraPathfinding &src);
	~DijkstraPathfinding() override;

	std::vector<std::shared_ptr<Node>> findPath(
		std::shared_ptr<Node> start, std::shared_ptr<Node> end) override;
};

#endif
