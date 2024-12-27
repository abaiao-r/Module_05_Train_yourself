/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simulation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 14:06:52 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/10/13 19:57:24 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "libraries.hpp"

class Simulation
{
	private:

	public:
		Simulation();
		Simulation(const Simulation &src);
		~Simulation();
		Simulation &operator=(const Simulation &src);
};

#endif
