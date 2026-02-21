/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#include <stdexcept>
#include <string>

class InputHandler
{
  private:
	std::string _railNetworkFilePath;
	std::string _trainFilePath;

  public:
	InputHandler(const std::string &railNetworkFile,
				 const std::string &trainFile);
	InputHandler(const InputHandler &src);
	InputHandler &operator=(const InputHandler &src);
	InputHandler(InputHandler &&src) noexcept;
	InputHandler &operator=(InputHandler &&src) noexcept;
	~InputHandler();

	void loadRailNetworkData();
	void loadTrainData();

	class InputException : public std::runtime_error
	{
	  public:
		explicit InputException(const std::string &msg);
	};
};

#endif
