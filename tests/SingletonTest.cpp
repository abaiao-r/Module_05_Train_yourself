/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SingletonTest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 15:06:53 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/10/17 14:46:20 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Singleton.hpp"

class MyClass : public Singleton<MyClass>
{
	public:
		void display()
		{
			std::cout << "MyClass Instance: " << this << std::endl;
		}
};

// Test 1: Basic Functionality Test
void testBasicFunctionality()
{
	MyClass &instance = Singleton<MyClass>::getInstance();
	instance.display();

	std::cout << GREEN << "Basic Functionality Test Passed!" << RESET
			  << std::endl;
}

// Test 2: Uniqueness Test
void testUniqueness()
{
	MyClass &instance1 = Singleton<MyClass>::getInstance();
	MyClass &instance2 = Singleton<MyClass>::getInstance();

	try
	{
		assert(&instance1 == &instance2);
		std::cout << GREEN
				  << "Uniqueness Test Passed: Both instances are the same!"
				  << RESET << std::endl;
	}
	catch (...)
	{
		std::cout << RED << "Uniqueness Test Failed: Instances are different!"
				  << RESET << std::endl;
	}
}

// Test 3: Thread Safety Test
void testThreadSafety()
{
	const int				 numThreads = 10;
	std::vector<std::thread> threads;
	MyClass				 *instances[numThreads];

	for (int i = 0; i < numThreads; ++i)
	{
		threads.emplace_back(
			[&, i]() { instances[i] = &Singleton<MyClass>::getInstance(); });
	}

	for (auto &t : threads)
	{
		t.join();
	}

	// Check if all instances point to the same address
	try
	{
		for (int i = 1; i < numThreads; ++i)
		{
			assert(instances[0] == instances[i]);
		}
		std::cout << GREEN
				  << "Thread Safety Test Passed: All instances are the same!"
				  << RESET << std::endl;
	}
	catch (...)
	{
		std::cout << RED
				  << "Thread Safety Test Failed: Instances are different!"
				  << RESET << std::endl;
	}
}

// Test 4: Copy and Move Constructor Test
void testCopyAndMoveConstructor()
{
	// Uncommenting the following lines should cause compilation errors
	// Singleton<MyClass> instance1;
	// Singleton<MyClass> instance2 = instance1; // Copy constructor
	// Singleton<MyClass> instance3 = std::move(instance1); // Move constructor
	std::cout
		<< YELLOW
		<< "To test Copy and Move Constructor, uncomment the lines in the test "
		   "function!\nIf compilation errors occur, the test is successful!"
		<< RESET << std::endl;
}

// Test 5: Assignment Operator Test
void testAssignmentOperator()
{
	// Uncommenting the following lines should cause compilation errors
	// Singleton<MyClass> instance1;
	// Singleton<MyClass> instance2;
	// instance2 = instance1; // Copy assignment
	// instance2 = std::move(instance1); // Move assignment
	std::cout
		<< YELLOW
		<< "To test Assignment Operator, uncomment the lines in the test "
		   "function!\nIf compilation errors occur, the test is successful!"
		<< RESET << std::endl;
}

int main(void)
{
	std::cout << "Running Basic Functionality Test..." << std::endl;
	testBasicFunctionality();

	std::cout << "\nRunning Uniqueness Test..." << std::endl;
	testUniqueness();

	std::cout << "\nRunning Thread Safety Test..." << std::endl;
	testThreadSafety();

	std::cout << "\nRunning Copy and Move Constructor Test..." << std::endl;
	testCopyAndMoveConstructor();

	std::cout << "\nRunning Assignment Operator Test..." << std::endl;
	testAssignmentOperator();

	return (0);
}