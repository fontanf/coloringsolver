#pragma once

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

struct ColumnGenerationOptionalParameters
{
    /** Linear programming solver. */
    std::string linear_programming_solver = "CLP";

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

Output columngenerationheuristic_greedy(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

Output columngenerationheuristic_limiteddiscrepancysearch(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

Output columngenerationheuristic_heuristictreesearch(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

}

