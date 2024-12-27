/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Singleton.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 14:12:08 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/10/17 14:53:22 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include "libraries.hpp"

template <typename T>
class Singleton
{
	private:
		static std::unique_ptr<T> instance;
		static std::once_flag	  initInstanceFlag;

	protected:
		// Private constructor to prevent instancing
		Singleton() = default;
		// Protected destructor to allow subclass destruction
		~Singleton() = default;

		// Initialize the instance
		static void initInstance()
		{
			instance.reset(new T());
		}

	public:
		// Delete copy and move constructors and assign operators
		Singleton(const Singleton &src) = delete;
		Singleton(Singleton &&src) = delete;
		Singleton &operator=(Singleton &&src) = delete;
		Singleton &operator=(const Singleton &src) = delete;

		// Get the instance of the singleton
		static T &getInstance()
		{
			std::call_once(initInstanceFlag, &Singleton::initInstance);

			return (*instance);
		}
};

template <typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr;

template <typename T>
std::once_flag Singleton<T>::initInstanceFlag;

#endif
