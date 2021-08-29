#pragma once

#include "coloringsolver/solution.hpp"

#include "coloringsolver/algorithms/greedy.hpp"
#include "coloringsolver/algorithms/localsearch.hpp"
#include "coloringsolver/algorithms/milp_cplex.hpp"
#include "coloringsolver/algorithms/columngeneration.hpp"

namespace coloringsolver
{

Output run(
        std::string algorithm,
        const Instance& instance,
        std::mt19937_64& generator,
        optimizationtools::Info info);

}

