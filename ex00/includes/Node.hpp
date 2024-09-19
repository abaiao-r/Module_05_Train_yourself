/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Node.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 13:45:42 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/18 18:05:46 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NODE_HPP
#define NODE_HPP

#include "libraries.hpp"

class Node
{
   private:
   public:
    Node(void);
    Node(const std::string &name);
    Node(const Node &src);
    ~Node(void);
    Node &operator=(const Node &src);
};

#endif
