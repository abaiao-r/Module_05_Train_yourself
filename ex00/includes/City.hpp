/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   City.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 13:48:35 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/19 03:49:51 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CITY_HPP
#define CITY_HPP

#include "libraries.hpp"

class City : public Node
{
	private:

	public:
		City();
		City(const City &src);
		~City();
		City &operator=(const City &src);
};

#endif
