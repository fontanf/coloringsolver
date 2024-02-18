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
            const Instance& instance):
        Output(instance) { }


    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

struct LocalSearchRowWeightingParameters: Parameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;

    /** Maximum number of improvements. */
    Counter maximum_number_of_improvements = -1;

    /** Goal. */
    ColorId goal = 0;

    /** Enable k-core reduction. */
    bool enable_core_reduction = true;

    /** Initial solution. */
    Solution* initial_solution = nullptr;
};

const LocalSearchRowWeightingOutput local_search_row_weighting(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeightingParameters& parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////// localsearch_rowweighting_2 //////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(
            const Instance& instance):
        Output(instance) { }


    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

struct LocalSearchRowWeighting2Parameters: Parameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;

    /** Maximum number of improvements. */
    Counter maximum_number_of_improvements = -1;

    /** Goal. */
    ColorId goal = 0;

    /** Enable k-core reduction. */
    bool enable_core_reduction = true;

    /** Initial solution. */
    Solution* initial_solution = nullptr;
};

const LocalSearchRowWeighting2Output local_search_row_weighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeighting2Parameters& parameters = {});

}
