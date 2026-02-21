/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IPathfindingAlgorithm.hpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IPATHFINDINGALGORITHM_HPP
#define IPATHFINDINGALGORITHM_HPP

#include <memory>
#include <vector>

class Node;

class IPathfindingAlgorithm
{
  public:
	virtual ~IPathfindingAlgorithm() = default;
	virtual std::vector<std::shared_ptr<Node>> findPath(
		std::shared_ptr<Node> start, std::shared_ptr<Node> end) = 0;
};

#endif
