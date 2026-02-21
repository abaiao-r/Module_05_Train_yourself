/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraPathfinding.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIJKSTRAPATHFINDING_HPP
#define DIJKSTRAPATHFINDING_HPP

#include "IPathfinding.hpp"

class DijkstraPathfinding : public IPathfinding
{
  public:
	DijkstraPathfinding();
	DijkstraPathfinding(const DijkstraPathfinding &);
	DijkstraPathfinding &operator=(const DijkstraPathfinding &);
	~DijkstraPathfinding() override;

	std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network) const override;
};

#endif
