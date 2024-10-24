/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DijkstraPathfinding.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 14:05:57 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 14:06:19 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIJKSTRAPATHFINDING_HPP
#define DIJKSTRAPATHFINDING_HPP

#include "libraries.hpp"

class DijkstraPathfinding : public IPathfindingAlgorithm
{
	private:

	public:
		DijkstraPathfinding();
		DijkstraPathfinding(const DijkstraPathfinding &src);
		virtual ~DijkstraPathfinding();
		DijkstraPathfinding &operator=(const DijkstraPathfinding &src);
};

#endif
