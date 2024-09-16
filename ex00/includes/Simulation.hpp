/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Simulation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 14:06:52 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 14:07:05 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATION_HPP
# define SIMULATION_HPP

# include "libraries.hpp"

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
