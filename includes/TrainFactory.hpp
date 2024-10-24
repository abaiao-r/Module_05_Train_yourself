/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TrainFactory.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 14:07:31 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 14:07:50 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRAINFACTORY_HPP
#define TRAINFACTORY_HPP

#include "libraries.hpp"

class TrainFactory
{
	private:

	public:
		TrainFactory();
		TrainFactory(const TrainFactory &src);
		~TrainFactory();
		TrainFactory &operator=(const TrainFactory &src);
};

#endif
