/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 21:54:26 by andrefranci       #+#    #+#             */
/*   Updated: 2024/09/16 16:37:17 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RAILNETWORK_HPP
# define RAILNETWORK_HPP

# include "libraries.hpp"

class RailNetwork
{
    private:

        
    public:
        RailNetwork();
        RailNetwork(const RailNetwork &src);
        ~RailNetwork();
        RailNetwork &operator=(const RailNetwork &src);
};

#endif
