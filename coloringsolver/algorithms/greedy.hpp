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

struct GreedyParameters: Parameters
{
    /** Ordering. */
    Ordering ordering = Ordering::DynamicLargestFirst;

    /** Reverse ordering. */
    bool reverse = false;
};

const Output greedy(
        const Instance& instance,
        const GreedyParameters& parameters = {});

const Output greedy_dsatur(
        const Instance& instance,
        const Parameters& parameters = {});

}
