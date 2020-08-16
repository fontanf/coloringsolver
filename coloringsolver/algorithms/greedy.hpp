#pragma once

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

Output greedy_dsatur(const Instance& instance, Info info = Info());

Output greedy_largestfirst(const Instance& instance, Info info = Info());

Output greedy_smallestlast(const Instance& instance, Info info = Info());

}

