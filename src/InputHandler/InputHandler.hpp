/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 17:21:05 by ctw03933         ###   ########.fr       */
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

	class ParseException : public std::runtime_error
	{
	  public:
		explicit ParseException(const std::string &msg);
	};

  private:
	static void parseNetworkFile(const std::string &filepath,
								 RailNetwork &network,
								 std::vector<Event> &events);
	static std::vector<std::unique_ptr<Train>> parseTrainFile(
		const std::string &filepath);
	static void validateData(const std::string &networkFile,
							 const std::string &trainFile,
							 const SimulationData &data);
};

#endif
