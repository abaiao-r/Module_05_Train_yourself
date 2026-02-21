/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GraphExporter.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 15:14:11 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GRAPHEXPORTER_HPP
#define GRAPHEXPORTER_HPP

#include <memory>
#include <string>
#include <vector>

class RailNetwork;
class Train;

/**
 * Generates a Graphviz DOT file that visualises the rail network
 * with each train's computed path highlighted in a unique colour.
 *
 * Usage:
 *   GraphExporter::exportDot("network.dot", network, trains);
 *   // then: dot -Tpng network.dot -o network.png
 */
class GraphExporter
{
  public:
	GraphExporter() = delete;

	static void exportDot(
		const std::string &filename,
		const RailNetwork &network,
		const std::vector<std::unique_ptr<Train>> &trains);
};

#endif
