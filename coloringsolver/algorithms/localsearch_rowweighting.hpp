#pragma once

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// localsearch_rowweighting ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeightingOutput: Output
{
    LocalSearchRowWeightingOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeightingOutput& algorithm_end(
            optimizationtools::Info& info);

    Counter number_of_iterations = 0;
};

using LocalSearchRowWeightingCallback = std::function<void(const LocalSearchRowWeightingOutput&)>;

struct LocalSearchRowWeightingOptionalParameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;
    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;
    /** Maximum number of improvements. */
    Counter maximum_number_of_improvements = -1;
    /** Enable k-core reduction. */
    bool enable_core_reduction = true;
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////// localsearch_rowweighting_2 //////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeighting2Output& algorithm_end(
            optimizationtools::Info& info);

    Counter number_of_iterations = 0;
};

using LocalSearchRowWeighting2Callback = std::function<void(const LocalSearchRowWeighting2Output&)>;

struct LocalSearchRowWeighting2OptionalParameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;
    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;
    /** Maximum number of improvements. */
    Counter maximum_number_of_improvements = -1;
    /** Enable k-core reduction. */
    bool enable_core_reduction = true;
    /** Initial solution. */
    Solution* initial_solution = nullptr;
    /** Callback function called when a new best solution is found. */
    LocalSearchRowWeighting2Callback new_solution_callback
        = [](const LocalSearchRowWeighting2Output&) { };
    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

LocalSearchRowWeighting2Output localsearch_rowweighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters = {});

}

