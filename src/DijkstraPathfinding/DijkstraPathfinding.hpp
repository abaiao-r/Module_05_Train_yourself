/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraPathfinding.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIJKSTRAPATHFINDING_HPP
#define DIJKSTRAPATHFINDING_HPP

#include "IPathfinding.hpp"

struct Edge;

class DijkstraPathfinding : public IPathfinding
{
  public:
	DijkstraPathfinding();
	DijkstraPathfinding(const DijkstraPathfinding &);
	DijkstraPathfinding &operator=(const DijkstraPathfinding &);
	~DijkstraPathfinding() override;

	std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network,
		PathWeightMode mode = PathWeightMode::Distance) const override;

  protected:
	static double edgeWeight(const Edge &edge, PathWeightMode mode);
};

#endif
