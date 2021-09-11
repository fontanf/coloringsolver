#pragma once

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

struct ColumnGenerationOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    std::string linear_programming_solver = "CLP";
};

/******************************************************************************/

struct ColumnGenerationHeuristicGreedyOutput: Output
{
    ColumnGenerationHeuristicGreedyOutput(const Instance& instance, optimizationtools::Info& info): Output(instance, info) { }
    ColumnGenerationHeuristicGreedyOutput& algorithm_end(optimizationtools::Info& info);

    std::vector<double> solution;
    std::vector<std::vector<double>> x;
};

ColumnGenerationHeuristicGreedyOutput columngenerationheuristic_greedy(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

/******************************************************************************/

struct ColumnGenerationHeuristicLimitedDiscrepancySearchOutput: Output
{
    ColumnGenerationHeuristicLimitedDiscrepancySearchOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    ColumnGenerationHeuristicLimitedDiscrepancySearchOutput& algorithm_end(
            optimizationtools::Info& info);

    std::vector<double> solution;
    std::vector<std::vector<double>> x;
};

ColumnGenerationHeuristicLimitedDiscrepancySearchOutput columngenerationheuristic_limiteddiscrepancysearch(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

/******************************************************************************/

struct ColumnGenerationHeuristicHeuristicTreeSearchOutput: Output
{
    ColumnGenerationHeuristicHeuristicTreeSearchOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    ColumnGenerationHeuristicHeuristicTreeSearchOutput& algorithm_end(
            optimizationtools::Info& info);

    std::vector<double> solution;
    std::vector<std::vector<double>> x;
};

ColumnGenerationHeuristicHeuristicTreeSearchOutput columngenerationheuristic_heuristictreesearch(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

}

