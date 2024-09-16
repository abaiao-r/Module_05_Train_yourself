/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 16:58:42 by andrefranci       #+#    #+#             */
/*   Updated: 2024/09/16 19:02:14 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/libraries.hpp"
#include "../includes/InputHandler.hpp"

int main(int argc, char **argv)
{
    // Check if the number of arguments is valid
    if (argc != 3)
    {
        std::cerr << RED << "Error: Invalid number of arguments" << RESET << std::endl;
        std::cerr << "Usage: ./pathfinder [railNetworkPrint] [trainPrint]" << std::endl;
        return (EXIT_FAILURE);
    }

    try
    {
        // Create an InputHandler object with the provided file paths
        InputHandler inputHandler(argv[1], argv[2]);

        // check validity of the first file(railNetworkPrint)
        inputHandler.loadRailNetworkData();

        // check validity of the second file(trainPrint)
        //inputHandler.loadTrainData();
        

    } catch (const InputHandler::InputException& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return (EXIT_SUCCESS);
}
