/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailSegment.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 13:50:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 13:50:49 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RAILSEGMENT_HPP
# define RAILSEGMENT_HPP

# include "libraries.hpp"

class RailSegment
{
    private:

    public:
        RailSegment();
        RailSegment(const RailSegment &src);
        ~RailSegment();
        RailSegment &operator=(const RailSegment &src);

};

#endif