/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AStarPathfinding.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 14:06:34 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 14:06:47 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ASTARPATHFINDING_HPP
#define ASTARPATHFINDING_HPP

#include "libraries.hpp"

class AStarPathfinding : public IPathfindingAlgorithm
{
	private:

	public:
		AStarPathfinding();
		AStarPathfinding(const AStarPathfinding &src);
		virtual ~AStarPathfinding();
		AStarPathfinding &operator=(const AStarPathfinding &src);
};

#endif
