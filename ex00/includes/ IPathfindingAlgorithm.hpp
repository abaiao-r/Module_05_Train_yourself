/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*    IPathfindingAlgorithm.hpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 14:03:46 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 14:04:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IPATHFINDINGALGORITHM_HPP
# define IPATHFINDINGALGORITHM_HPP

# include "libraries.hpp"

class IPathfindingAlgorithm
{
    private:

    public:
        IPathfindingAlgorithm();
        IPathfindingAlgorithm(const IPathfindingAlgorithm &src);
        virtual ~IPathfindingAlgorithm();
        IPathfindingAlgorithm &operator=(const IPathfindingAlgorithm &src);

};

#endif
