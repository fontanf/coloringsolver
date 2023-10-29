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

Output column_generation_heuristic_greedy(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

Output column_generation_heuristic_limited_discrepancy_search(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

Output column_generation_heuristic_heuristic_tree_search(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

}

