/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Event.hpp"
#include "RailNetwork.hpp"
#include "Train.hpp"

struct SimulationData
{
	RailNetwork network;
	std::vector<std::unique_ptr<Train>> trains;
	std::vector<Event> events;
};

class InputHandler
{
  public:
	InputHandler() = delete;

	static SimulationData loadData(const std::string &networkFile,
								   const std::string &trainFile);

	/** Parse a network file only (nodes, rails, events). No cross-validation. */
	static void parseNetworkFile(const std::string &filepath,
								 RailNetwork &network,
								 std::vector<Event> &events);

	/** Parse a train file only.  No station-existence validation. */
	static std::vector<std::unique_ptr<Train>> parseTrainFile(
		const std::string &filepath);

	class ParseException : public std::runtime_error
	{
	  public:
		explicit ParseException(const std::string &msg);
	};

  private:
	static void validateData(const std::string &networkFile,
							 const std::string &trainFile,
							 const SimulationData &data);
};

#endif
