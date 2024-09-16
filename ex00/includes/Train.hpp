/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Train.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 13:51:45 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 13:51:58 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRAIN_HPP
# define TRAIN_HPP

# include "libraries.hpp"

class Train
{
    private:

    public:
        Train();
        Train(const Train &src);
        ~Train();
        Train &operator=(const Train &src);

};

#endif
