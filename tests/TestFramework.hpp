/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestFramework.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r           #+#    #+#             */
/*   Updated: 2026/02/21 02:45:00 by abaiao-r          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TESTFRAMEWORK_HPP
#define TESTFRAMEWORK_HPP

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define CLR_RED "\033[31m"
#define CLR_GREEN "\033[32m"
#define CLR_YELLOW "\033[33m"
#define CLR_RESET "\033[0m"

#define ASSERT_TRUE(expr, msg)                                     \
	do                                                             \
	{                                                              \
		if (!(expr))                                               \
		{                                                          \
			msg = std::string("Expected true: ") + #expr;         \
			return false;                                          \
		}                                                          \
	} while (0)

#define ASSERT_FALSE(expr, msg)                                    \
	do                                                             \
	{                                                              \
		if ((expr))                                                \
		{                                                          \
			msg = std::string("Expected false: ") + #expr;        \
			return false;                                          \
		}                                                          \
	} while (0)

#define ASSERT_EQ(expected, actual, msg)                           \
	do                                                             \
	{                                                              \
		if ((expected) != (actual))                                \
		{                                                          \
			std::ostringstream oss;                                \
			oss << "Expected " << (expected) << " but got "       \
				<< (actual);                                      \
			msg = oss.str();                                      \
			return false;                                          \
		}                                                          \
	} while (0)

#define ASSERT_STR_EQ(expected, actual, msg)                       \
	do                                                             \
	{                                                              \
		if ((expected) != (actual))                                \
		{                                                          \
			msg = "Expected \"" + std::string(expected)           \
				  + "\" but got \"" + std::string(actual) + "\""; \
			return false;                                          \
		}                                                          \
	} while (0)

#define ASSERT_THROWS(expr, exType, msg)                           \
	do                                                             \
	{                                                              \
		bool caught = false;                                       \
		try                                                        \
		{                                                          \
			expr;                                                  \
		}                                                          \
		catch (const exType &)                                     \
		{                                                          \
			caught = true;                                         \
		}                                                          \
		catch (...)                                                \
		{                                                          \
		}                                                          \
		if (!caught)                                               \
		{                                                          \
			msg = std::string("Expected ") + #exType;             \
			return false;                                          \
		}                                                          \
	} while (0)

namespace Test
{

class TestSuite
{
  private:
	std::string _name;
	int _passed;
	int _failed;

  public:
	explicit TestSuite(const std::string &name)
		: _name(name), _passed(0), _failed(0)
	{
		std::cout << "\n========================================"
				  << std::endl;
		std::cout << "  " << _name << " Tests" << std::endl;
		std::cout << "========================================\n"
				  << std::endl;
	}

	using TestFunc = std::function<bool(std::string &)>;

	void run(const std::string &testName, TestFunc fn)
	{
		std::string msg;
		try
		{
			if (fn(msg))
			{
				std::cout << "  " CLR_GREEN "[PASS]" CLR_RESET " "
						  << testName << std::endl;
				_passed++;
			}
			else
			{
				std::cout << "  " CLR_RED "[FAIL]" CLR_RESET " "
						  << testName << ": " << msg << std::endl;
				_failed++;
			}
		}
		catch (const std::exception &e)
		{
			std::cout << "  " CLR_RED "[FAIL]" CLR_RESET " " << testName
					  << ": Exception: " << e.what() << std::endl;
			_failed++;
		}
	}

	int summarize()
	{
		std::cout << "\n----------------------------------------"
				  << std::endl;
		std::cout << "  Summary: " << _passed << " passed, " << _failed
				  << " failed" << std::endl;
		std::cout << "----------------------------------------\n"
				  << std::endl;
		return _failed > 0 ? 1 : 0;
	}
};

}  // namespace Test

#endif
