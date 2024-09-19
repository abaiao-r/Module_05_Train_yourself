/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IMediator.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 17:10:53 by andrefranci       #+#    #+#             */
/*   Updated: 2024/09/19 03:58:44 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IMEDIATOR_HPP
#define IMEDIATOR_HPP

#include "libraries.hpp"

class IMediator
{
	private:
		int						 _id;
		int const				 _jumps;
		std::vector<IObserver *> _observers;

	public:
		virtual void notify(std::string const &message) const = 0;
};

#endif
