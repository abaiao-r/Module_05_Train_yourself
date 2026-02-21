/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OutputManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef OUTPUTMANAGER_HPP
#define OUTPUTMANAGER_HPP

#include <string>

class OutputManager
{
  public:
	OutputManager();
	OutputManager(const OutputManager &src);
	OutputManager &operator=(const OutputManager &src);
	~OutputManager();
};

#endif
