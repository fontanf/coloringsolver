#pragma once

/**
 * Local Search:
 */

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

struct LocalSearchOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    /** Maximum number of nodes. */
    Counter maximum_number_of_nodes = -1;

    /** Number of threads. */
    Counter number_of_threads = 1;

    /** Initial solution (facultative). */
    const Solution* initial_solution = nullptr;
};

struct LocalSearchOutput: Output
{
    LocalSearchOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchOutput& algorithm_end(
            optimizationtools::Info& info);
};

LocalSearchOutput localsearch(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchOptionalParameters parameters = {});

}

