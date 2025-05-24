/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 21:54:26 by andrefranci       #+#    #+#             */
/*   Updated: 2025/05/24 14:42:34 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RAILNETWORK_HPP
#define RAILNETWORK_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Edge.hpp"
#include "Node.hpp"
#include "Singleton.hpp"

class RailNetwork : public Singleton<RailNetwork>
{
	private:
		std::unordered_map<std::string, std::shared_ptr<Node>> _nodes;

	public:
		RailNetwork();
		~RailNetwork();

		void				  addNode(const std::shared_ptr<Node> &node);
		std::shared_ptr<Node> getNode(const std::string &name) const;
		const std::unordered_map<std::string, std::shared_ptr<Node>> &getNodes()
			const;
		std::vector<Edge> getAllEdges() const;

		void printNetwork() const;
};

#endif
