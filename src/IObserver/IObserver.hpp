/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IObserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IOBSERVER_HPP
#define IOBSERVER_HPP

#include <string>

class IObserver
{
  public:
	virtual ~IObserver() = default;
	virtual void update(const std::string &event,
						const std::string &message) = 0;
};

#endif
