/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestFramework.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 01:31:16 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/21 01:31:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace Test
{

const std::string CLR_RED = "\033[31m";
const std::string CLR_GREEN = "\033[32m";
const std::string CLR_BLUE = "\033[34m";
const std::string CLR_RESET = "\033[0m";

struct TestResult
{
	std::string name;
	bool passed;
	std::string message;
};

class TestSuite
{
  private:
	std::string _suiteName;
	std::vector<TestResult> _results;
	int _passed = 0;
	int _failed = 0;

  public:
	explicit TestSuite(const std::string &name) : _suiteName(name)
	{
		std::cout << CLR_BLUE << "\n========================================\n"
				  << "  " << _suiteName << " Tests\n"
				  << "========================================" << CLR_RESET
				  << "\n\n";
	}

	void run(const std::string &testName,
			 const std::function<bool(std::string &)> &testFunc)
	{
		std::string message;
		bool passed = false;

		try
		{
			passed = testFunc(message);
		}
		catch (const std::exception &e)
		{
			message = std::string("Exception: ") + e.what();
			passed = false;
		}
		catch (...)
		{
			message = "Unknown exception";
			passed = false;
		}

		_results.push_back({testName, passed, message});

		if (passed)
		{
			_passed++;
			std::cout << CLR_GREEN << "  [PASS] " << testName << CLR_RESET
					  << "\n";
		}
		else
		{
			_failed++;
			std::cout << CLR_RED << "  [FAIL] " << testName << CLR_RESET
					  << "\n";
			if (!message.empty())
				std::cout << "         " << message << "\n";
		}
	}

	int summarize()
	{
		std::cout << "\n"
				  << CLR_BLUE
				  << "----------------------------------------\n"
				  << "  Summary: " << CLR_RESET << CLR_GREEN << _passed
				  << " passed" << CLR_RESET << ", "
				  << (_failed > 0 ? CLR_RED : CLR_GREEN) << _failed
				  << " failed" << CLR_RESET << "\n"
				  << CLR_BLUE
				  << "----------------------------------------" << CLR_RESET
				  << "\n\n";
		return _failed > 0 ? 1 : 0;
	}
};

/* ---- Assertion macros ---- */

#define ASSERT_TRUE(expr, msg)                                                 \
	do                                                                         \
	{                                                                          \
		if (!(expr))                                                           \
		{                                                                      \
			msg = std::string("Expected true: ") + #expr;                      \
			return false;                                                      \
		}                                                                      \
	} while (0)

#define ASSERT_FALSE(expr, msg)                                                \
	do                                                                         \
	{                                                                          \
		if ((expr))                                                            \
		{                                                                      \
			msg = std::string("Expected false: ") + #expr;                     \
			return false;                                                      \
		}                                                                      \
	} while (0)

#define ASSERT_EQ(expected, actual, msg)                                        \
	do                                                                         \
	{                                                                          \
		if ((expected) != (actual))                                             \
		{                                                                      \
			msg = std::string("Expected ") + std::to_string(expected)          \
				  + " but got " + std::to_string(actual);                      \
			return false;                                                      \
		}                                                                      \
	} while (0)

#define ASSERT_STR_EQ(expected, actual, msg)                                    \
	do                                                                         \
	{                                                                          \
		if ((expected) != (actual))                                             \
		{                                                                      \
			msg = std::string("Expected '") + (expected) + "' but got '"       \
				  + (actual) + "'";                                            \
			return false;                                                      \
		}                                                                      \
	} while (0)

#define ASSERT_THROWS(expr, exceptionType, msg)                                \
	do                                                                         \
	{                                                                          \
		bool caught = false;                                                   \
		try                                                                    \
		{                                                                      \
			expr;                                                              \
		}                                                                      \
		catch (const exceptionType &)                                          \
		{                                                                      \
			caught = true;                                                     \
		}                                                                      \
		catch (...)                                                            \
		{                                                                      \
		}                                                                      \
		if (!caught)                                                           \
		{                                                                      \
			msg = std::string("Expected ") + #exceptionType                    \
				  + " to be thrown";                                           \
			return false;                                                      \
		}                                                                      \
	} while (0)

}  // namespace Test

#endif
