/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
#define NODE_HPP

#include <stdexcept>
#include <string>

class Node
{
  private:
	std::string _name;

  public:
	explicit Node(const std::string &name);
	Node(const Node &src);
	Node &operator=(const Node &src);
	~Node();

	const std::string &getName() const;

	class InvalidNameException : public std::invalid_argument
	{
	  public:
		explicit InvalidNameException(const std::string &msg);
	};
};

#endif
