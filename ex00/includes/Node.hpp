/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 13:45:42 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 13:49:04 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
# define NODE_HPP

# include "libraries.hpp"

class Node
{
    private:


    public:
        Node();
        Node(const std::string &name);
        Node(const Node &src);
        ~Node();
        Node &operator=(const Node &src);

};

#endif
