/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IPathfinding.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r           #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IPATHFINDING_HPP
#define IPATHFINDING_HPP

#include <memory>
#include <string>
#include <vector>

class Node;
class RailNetwork;

class IPathfinding
{
  public:
	virtual ~IPathfinding() = default;

	virtual std::vector<std::shared_ptr<Node>> findPath(
		const std::string &start, const std::string &end,
		const RailNetwork &network) const = 0;
};

#endif
