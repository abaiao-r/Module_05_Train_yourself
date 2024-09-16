/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OutputManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 14:08:05 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 14:08:22 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef OUTPUTMANAGER_HPP
# define OUTPUTMANAGER_HPP

# include "libraries.hpp"

class OutputManager
{
    private:

    public:
        OutputManager();
        OutputManager(const OutputManager &src);
        ~OutputManager();
        OutputManager &operator=(const OutputManager &src);

};

#endif
