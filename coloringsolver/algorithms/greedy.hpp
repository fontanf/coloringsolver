#pragma once

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

enum class Ordering {
    Default,
    LargestFirst,
    IncidenceDegree,
    SmallestLast,
    DynamicLargestFirst,
};

std::istream& operator>>(std::istream& in, Ordering& problem_type);
std::ostream& operator<<(std::ostream &os, Ordering ordering);

struct GreedyOptionalParameters
{
    Ordering ordering = Ordering::DynamicLargestFirst;
    bool reverse = false;

    optimizationtools::Info info = optimizationtools::Info();
};

Output greedy(
        const Instance& instance,
        GreedyOptionalParameters = {});

Output greedy_dsatur(
        const Instance& instance,
        optimizationtools::Info info = optimizationtools::Info());

}

