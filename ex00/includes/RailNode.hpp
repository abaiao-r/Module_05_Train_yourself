/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNode.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 13:49:56 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 13:50:09 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RAILNODE_HPP
# define RAILNODE_HPP

# include "libraries.hpp"

class RailNode : public Node
{
    private:

    public:
        RailNode();
        RailNode(const RailNode &src);
        ~RailNode();
        RailNode &operator=(const RailNode &src);

};

#endif
