#pragma once

#include "coloringsolver/solution.hpp"

#include "coloringsolver/algorithms/greedy.hpp"
#include "coloringsolver/algorithms/branchandcut_cplex.hpp"

namespace coloringsolver
{

Output run(std::string algorithm, const Instance& instance, std::mt19937_64& generator, Info info);

}

