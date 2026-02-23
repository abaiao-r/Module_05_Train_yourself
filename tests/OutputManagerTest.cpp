/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OutputManagerTest.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "OutputManager.hpp"
#include "TestFramework.hpp"

int main()
{
	Test::TestSuite suite("OutputManager");

	suite.run("formatTime midnight", [](std::string &msg) {
		ASSERT_STR_EQ(std::string("00:00:00"),
					  OutputManager::formatTime(0.0), msg);
		return true;
	});

	suite.run("formatTime one hour one minute one second",
			  [](std::string &msg) {
				  ASSERT_STR_EQ(std::string("01:01:01"),
								OutputManager::formatTime(3661.0), msg);
				  return true;
			  });

	suite.run("formatTime end of day", [](std::string &msg) {
		ASSERT_STR_EQ(std::string("23:59:59"),
					  OutputManager::formatTime(86399.0), msg);
		return true;
	});

	suite.run("formatTime wraps at 24h", [](std::string &msg) {
		ASSERT_STR_EQ(std::string("00:00:00"),
					  OutputManager::formatTime(86400.0), msg);
		return true;
	});

	suite.run("formatTime 14h10", [](std::string &msg) {
		ASSERT_STR_EQ(std::string("14:10:00"),
					  OutputManager::formatTime(51000.0), msg);
		return true;
	});

	return suite.summarize();
}
