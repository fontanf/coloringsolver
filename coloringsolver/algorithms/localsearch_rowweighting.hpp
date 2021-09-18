#pragma once

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

struct LocalSearchRowWeightingOutput: Output
{
    LocalSearchRowWeightingOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeightingOutput& algorithm_end(
            optimizationtools::Info& info);
};

using LocalSearchRowWeightingCallback = std::function<void(const LocalSearchRowWeightingOutput&)>;

struct LocalSearchRowWeightingOptionalParameters
{
    /** Number of threads. */
    Counter number_of_threads = 3;
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;
    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;
    /** Maximum number of improvements. */
    Counter maximum_number_of_improvements = -1;
    /** Initial solution. */
    Solution* initial_solution = nullptr;
    /** Callback function called when a new best solution is found. */
    LocalSearchRowWeightingCallback new_solution_callback
        = [](const LocalSearchRowWeightingOutput&) { };
    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

LocalSearchRowWeightingOutput localsearch_rowweighting(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeightingOptionalParameters parameters = {});

}

