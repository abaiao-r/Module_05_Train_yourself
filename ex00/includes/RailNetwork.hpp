/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 21:54:26 by andrefranci       #+#    #+#             */
/*   Updated: 2024/09/20 12:04:33 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RAILNETWORK_HPP
#define RAILNETWORK_HPP

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Node.hpp"
#include "libraries.hpp"

class RailNetwork
{
	private:
		using AdjacencyList = std::unordered_map<
			std::shared_ptr<Node>,
			std::vector<std::pair<std::shared_ptr<Node>, int>>>;
		AdjacencyList _adjacencyList;

	public:
		RailNetwork();
		RailNetwork(const RailNetwork &src);
		~RailNetwork();
		RailNetwork &operator=(const RailNetwork &src);
};

#endif
