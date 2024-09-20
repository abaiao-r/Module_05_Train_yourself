/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IMediator.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 17:10:53 by andrefranci       #+#    #+#             */
/*   Updated: 2024/09/20 12:08:25 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IMEDIATOR_HPP
#define IMEDIATOR_HPP

#include "libraries.hpp"

class IMediator
{
	private:
		std::vector<IObserver *> _observers;

	public:
		virtual void notify(std::string const &Event, std::string const &Message) = 0;
};

#endif
