#pragma once

#include "coloringsolver/solution.hpp"

#include "coloringsolver/algorithms/greedy.hpp"
#include "coloringsolver/algorithms/local_search_row_weighting.hpp"
#include "coloringsolver/algorithms/milp_cplex.hpp"
#include "coloringsolver/algorithms/column_generation.hpp"

namespace coloringsolver
{

Output run(
        std::string algorithm,
        const Instance& instance,
        const Solution& initial_solution,
        ColorId goal,
        std::mt19937_64& generator,
        optimizationtools::Info info);

}

