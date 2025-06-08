#pragma once

#include "coloringsolver/solution.hpp"

#include "columngenerationsolver/commons.hpp"

namespace coloringsolver
{

struct ColumnGenerationParameters: Parameters
{
    /** Linear programming solver. */
    columngenerationsolver::SolverName linear_programming_solver = columngenerationsolver::SolverName::CLP;
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
