#pragma once

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

struct ColumnGenerationParameters: Parameters
{
    /** Linear programming solver. */
    std::string linear_programming_solver = "CLP";
};

const Output column_generation_heuristic_greedy(
        const Instance& instance,
        const ColumnGenerationParameters& parameters = {});

const Output column_generation_heuristic_limited_discrepancy_search(
        const Instance& instance,
        const ColumnGenerationParameters& parameters = {});

const Output column_generation_heuristic_heuristic_tree_search(
        const Instance& instance,
        const ColumnGenerationParameters& parameters = {});

}
