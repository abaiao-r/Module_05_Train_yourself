/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventManager.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 13:51:26 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 15:25:06 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTMANAGER_HPP
# define EVENTMANAGER_HPP

# include "libraries.hpp"

//singleton
class EventManager
{
    private:

    public:
        EventManager();
        EventManager(const EventManager &src);
        ~EventManager();
        EventManager &operator=(const EventManager &src);

};

#endif

