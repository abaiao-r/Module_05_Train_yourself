/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 16:48:17 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/10/24 10:54:27 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#include "libraries.hpp"

class InputHandler
{
	private:
		std::string railNetworkFilePath;
		std::string trainPrintFilePath;
		bool		nodeStr;
		bool		eventStr;
		bool		railStr;

	public:
		InputHandler(const std::string &railNetworkPrintFilePath,
					 const std::string &trainPrintFilePathParam);
		InputHandler(const InputHandler &src);
		InputHandler &operator=(const InputHandler &src);
		InputHandler(InputHandler &&src);
		InputHandler &operator=(InputHandler &&src);
		~InputHandler();

		// Custom exception class
		class InputException : public std::exception
		{
			private:
				std::string message;

			public:
				explicit InputException(const std::string &msg) : message(msg)
				{
				}

				virtual const char *what() const noexcept override
				{
					return message.c_str();
				}
		};

		// methods
		void loadRailNetworkData();
		void loadTrainData();
};

#endif
