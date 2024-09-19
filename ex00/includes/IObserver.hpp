/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IObserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 14:09:49 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/19 03:49:16 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IOBSERVER_HPP
#define IOBSERVER_HPP

#include "libraries.hpp"

class IObserver
{
	private:

	public:
		IObserver();
		IObserver(const IObserver &src);
		virtual ~IObserver();
		IObserver &operator=(const IObserver &src);

		virtual void update() = 0;
};

#endif
