/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Singleton.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <memory>
#include <mutex>

template <typename T>
class Singleton
{
  private:
	static std::unique_ptr<T> _instance;
	static std::once_flag _initFlag;

  protected:
	Singleton() = default;
	~Singleton() = default;

	static void _initInstance() { _instance.reset(new T()); }

  public:
	Singleton(const Singleton &) = delete;
	Singleton(Singleton &&) = delete;
	Singleton &operator=(const Singleton &) = delete;
	Singleton &operator=(Singleton &&) = delete;

	static T &getInstance()
	{
		std::call_once(_initFlag, &Singleton::_initInstance);
		return *_instance;
	}
};

template <typename T>
std::unique_ptr<T> Singleton<T>::_instance = nullptr;

template <typename T>
std::once_flag Singleton<T>::_initFlag;

#endif
