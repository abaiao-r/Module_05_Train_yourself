/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Edge.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EDGE_HPP
#define EDGE_HPP

#include <memory>

class Node;

struct Edge
{
	std::weak_ptr<Node> node;
	double distance;
	double speedLimit;
};

#endif
