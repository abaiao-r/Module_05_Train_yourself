/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InputHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 17:36:05 by abaiao-r          #+#    #+#             */
/*   Updated: 2024/09/16 19:45:19 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/InputHandler.hpp"

// Parameterized constructor
InputHandler::InputHandler(const std::string &railNetworkPrintFilePath, const std::string &trainPrintFilePath)
    : railNetworkFilePath(railNetworkPrintFilePath), trainPrintFilePath(trainPrintFilePath)
{
}

// Copy constructor
InputHandler::InputHandler(const InputHandler &src)
{
    *this = src;
}

// Assignment operator
InputHandler &InputHandler::operator=(const InputHandler &src)
{
    if (this != &src)
    {
        this->railNetworkFilePath = src.railNetworkFilePath;
        this->trainPrintFilePath = src.trainPrintFilePath;
    }
    return (*this);
}

// Move constructor
InputHandler::InputHandler(InputHandler &&src)
{
    *this = std::move(src);
}

// Move assignment operator
InputHandler &InputHandler::operator=(InputHandler &&src)
{
    if (this != &src)
    {
        this->railNetworkFilePath = src.railNetworkFilePath;
        this->trainPrintFilePath = src.trainPrintFilePath;
    }
    return (*this);
}

// Destructor
InputHandler::~InputHandler()
{
}

// Load rail network data
void InputHandler::loadRailNetworkData()
{
    std::ifstream file(railNetworkFilePath);
    if (!file.is_open())
    {
        throw InputException("Failed to open file: " + railNetworkFilePath);
    }
    // check if the file is empty
    if (file.peek() == std::ifstream::traits_type::eof())
    {
        throw InputException("File is empty: " + railNetworkFilePath);
    }
    // check if the file is valid
   // go through the file line by line,
    // the first word of each line should be "Node", "Event", or "Rail" not case sensitive
    // if the first word is not one of the above, throw an exception
    // also if the first word is "Node", the line should have 2 words, the first one is "Node" and the second one is the node name
    // if the first word is "Event", the line should have 5 words, the first one is "Event", the second one is the event name(it can be a single word or group of word inside quotes ex: Riot, or "There is something"), and the third is a double type reenting  probability, the fourth duration of the event in time units, and the fifth is the node name
    // if the first word is "Rail", the line should have 4 words, the first one is "Rail", the second one is where the rail starts, the third is where the rail ends, and the fourth is the length of the rail in time units in double type
    
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string word;
        iss >> word;
        if (word == "Node")
        {
            if (nodeStr)
            {
                throw InputException("Invalid file format: " + railNetworkFilePath);
            }
            nodeStr = true;
            std::string nodeName;
            iss >> nodeName;
            if (nodeName.empty())
            {
                throw InputException("Invalid file format: " + railNetworkFilePath);
            }
        }
        else if (word == "Event")
        {
            if (eventStr)
            {
                throw InputException("Invalid file format: " + railNetworkFilePath);
            }
            eventStr = true;
            std::string eventName;
            double probability;
            int duration;
            std::string nodeName;
            iss >> eventName >> probability >> duration >> nodeName;
            if (eventName.empty() || nodeName.empty())
            {
                throw InputException("Invalid file format: " + railNetworkFilePath);
            }
        }
        else if (word == "Rail")
        {
            if (railStr)
            {
                throw InputException("Invalid file format: " + railNetworkFilePath);
            }
            railStr = true;
            std::string startNode;
            std::string endNode;
            double length;
            iss >> startNode >> endNode >> length;
            if (startNode.empty() || endNode.empty())
            {
                throw InputException("Invalid file format: " + railNetworkFilePath);
            }
        }
        else
        {
            throw InputException("Invalid file format: " + railNetworkFilePath);
        }
    }
    
    
    
    
    file.close();
}


