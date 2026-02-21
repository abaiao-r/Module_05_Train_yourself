/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GraphExporter.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 15:35:20 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GraphExporter.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

#include "Node.hpp"
#include "RailNetwork.hpp"
#include "Train.hpp"

/* ---- colour palette (8 distinct colours for up to 8 trains) ---- */
static const char *TRAIN_COLOURS[] = {
	"#e6194b", // red
	"#3cb44b", // green
	"#4363d8", // blue
	"#f58231", // orange
	"#911eb4", // purple
	"#42d4f4", // cyan
	"#f032e6", // magenta
	"#bfef45", // lime
};
static constexpr size_t NUM_COLOURS = sizeof(TRAIN_COLOURS) / sizeof(TRAIN_COLOURS[0]);

/* ---- helpers ---- */

/** Build a canonical key for an undirected edge (alphabetical order). */
static std::string edgeKey(const std::string &a, const std::string &b)
{
	return (a < b) ? a + "|" + b : b + "|" + a;
}

/* ---- public ---- */
void GraphExporter::exportDot(
	const std::string &filename,
	const RailNetwork &network,
	const std::vector<std::unique_ptr<Train>> &trains)
{
	std::ofstream out(filename);
	if (!out.is_open())
		throw std::runtime_error("Cannot create graph file: " + filename);

	out << "graph RailNetwork {\n";
	out << "    rankdir=LR;\n";
	out << "    bgcolor=\"#1e1e2e\";\n";
	out << "    pad=0.5;\n";
	out << "    nodesep=0.8;\n";
	out << "    ranksep=1.2;\n";
	out << "    fontname=\"Helvetica\";\n";
	out << "    label=<<b>Rail Network — Train Paths</b>>;\n";
	out << "    labelloc=t;\n";
	out << "    fontsize=18;\n";
	out << "    fontcolor=\"#cdd6f4\";\n\n";

	/* Node styling */
	out << "    node [\n";
	out << "        shape=box, style=\"filled,rounded\",\n";
	out << "        fillcolor=\"#313244\", fontcolor=\"#cdd6f4\",\n";
	out << "        color=\"#585b70\", fontname=\"Helvetica\",\n";
	out << "        fontsize=11, margin=\"0.15,0.08\"\n";
	out << "    ];\n\n";

	/* Default edge style */
	out << "    edge [\n";
	out << "        color=\"#585b70\", fontcolor=\"#a6adc8\",\n";
	out << "        fontname=\"Helvetica\", fontsize=9\n";
	out << "    ];\n\n";

	/* Emit all nodes — highlight stations (City*) differently */
	auto nodeNames = network.getNodeNames();
	for (const auto &name : nodeNames)
	{
		bool isCity = (name.substr(0, 4) == "City");
		if (isCity)
			out << "    \"" << name
				<< "\" [fillcolor=\"#89b4fa\", fontcolor=\"#1e1e2e\","
				<< " style=\"filled,rounded,bold\", penwidth=2];\n";
		else
			out << "    \"" << name << "\";\n";
	}
	out << "\n";

	/* Collect all edges (undirected — avoid duplicates) */
	std::set<std::string> emitted;
	for (const auto &nodeName : nodeNames)
	{
		for (const auto &edge : network.getNeighbours(nodeName))
		{
			const std::string &dest = edge.destination->getName();
			std::string key = edgeKey(nodeName, dest);
			if (emitted.count(key))
				continue;
			emitted.insert(key);

			char label[64];
			std::snprintf(label, sizeof(label), "%.1f km\\n%.0f km/h",
						  edge.distance, edge.speedLimit);
			out << "    \"" << nodeName << "\" -- \"" << dest
				<< "\" [label=\"" << label << "\"];\n";
		}
	}
	out << "\n";

	/* Overlay each train's path in colour */
	for (size_t t = 0; t < trains.size(); t++)
	{
		const auto &path = trains[t]->getPath();
		if (path.size() < 2)
			continue;

		const char *colour = TRAIN_COLOURS[t % NUM_COLOURS];

		/* Legend entry: invisible node + label */
		out << "    \"_legend_" << t
			<< "\" [label=\"" << trains[t]->getName()
			<< "\", shape=plaintext, fontcolor=\"" << colour
			<< "\", fontsize=12, fontname=\"Helvetica Bold\"];\n";

		for (size_t i = 0; i + 1 < path.size(); i++)
		{
			const std::string &from = path[i]->getName();
			const std::string &to = path[i + 1]->getName();

			/* Look up the edge to annotate speed */
			double speed = 0.0;
			for (const auto &edge : network.getNeighbours(from))
			{
				if (edge.destination->getName() == to)
				{
					speed = edge.speedLimit;
					break;
				}
			}

			out << "    \"" << from << "\" -- \"" << to
				<< "\" [color=\"" << colour
				<< "\", penwidth=3, style=bold";
			if (speed > 0.0)
			{
				char speedLabel[32];
				std::snprintf(speedLabel, sizeof(speedLabel),
							  "%.0f km/h", speed);
				out << ", label=\"" << speedLabel << "\","
					<< " fontcolor=\"" << colour << "\"";
			}
			out << "];\n";
		}
		out << "\n";
	}

	out << "}\n";
	out.close();

	std::cout << "Graph exported to " << filename << std::endl;
	std::cout << "Render with: dot -Tpng " << filename
			  << " -o output/graphs/network.png" << std::endl;
}
