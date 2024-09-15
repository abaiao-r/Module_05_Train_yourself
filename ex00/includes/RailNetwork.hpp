/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RailNetwork.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andrefrancisco <andrefrancisco@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 21:54:26 by andrefranci       #+#    #+#             */
/*   Updated: 2024/09/15 22:35:16 by andrefranci      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RAILNETWORK_HPP
# define RAILNETWORK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <queue>
#include <stack>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <utility>
#include <memory>
#include <stdexcept>
#include <exception>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cinttypes>
#include <cstdbool>
#include <cstdalign>


class Node;
class RailSegment;
class RailNetwork;

class ParseInput
{
    private:

    public:
        ParseInput();
        ParseInput(const ParseInput &src);
        ~ParseInput();
        ParseInput &operator=(const ParseInput &src);
};

class Event
{
    private:

    public:
        Event();
        Event(const Event &src);
        ~Event();
        Event &operator=(const Event &src);
};

class Train
{
    private:

    public:
        Train();
        Train(const Train &src);
        ~Train();
        Train &operator=(const Train &src);
};

class Node
{
    private:

    public:
        Node();
        Node(const Node &src);
        ~Node();
        Node &operator=(const Node &src);
};

class RailSegment
{
    private:

        
    public:
        RailSegment();
        RailSegment(const RailSegment &src);
        ~RailSegment();
        RailSegment &operator=(const RailSegment &src);
};



class RailNetwork
{
    private:

        
    public:
        RailNetwork();
        RailNetwork(const RailNetwork &src);
        ~RailNetwork();
        RailNetwork &operator=(const RailNetwork &src);
};

#endif
