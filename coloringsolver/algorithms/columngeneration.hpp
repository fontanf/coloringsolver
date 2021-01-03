#pragma once

#include "coloringsolver/solution.hpp"

#include "columngenerationsolver/columngenerationsolver.hpp"

#include "stablesolver/algorithms/largeneighborhoodsearch.hpp"

namespace coloringsolver
{

typedef columngenerationsolver::RowIdx RowIdx;
typedef columngenerationsolver::ColIdx ColIdx;
typedef columngenerationsolver::Value Value;
typedef columngenerationsolver::Column Column;
typedef columngenerationsolver::LinearProgrammingSolver LinearProgrammingSolver;

struct ColumnGenerationOptionalParameters
{
    Info info = Info();

    LinearProgrammingSolver linear_programming_solver = LinearProgrammingSolver::CLP;
};

/******************************************************************************/

struct ColumnGenerationHeuristicGreedyOutput: Output
{
    ColumnGenerationHeuristicGreedyOutput(const Instance& instance, Info& info): Output(instance, info) { }
    ColumnGenerationHeuristicGreedyOutput& algorithm_end(Info& info);

    std::vector<double> solution;
    std::vector<std::vector<double>> x;
};

ColumnGenerationHeuristicGreedyOutput columngenerationheuristic_greedy(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

/******************************************************************************/

struct ColumnGenerationHeuristicLimitedDiscrepancySearchOutput: Output
{
    ColumnGenerationHeuristicLimitedDiscrepancySearchOutput(const Instance& instance, Info& info): Output(instance, info) { }
    ColumnGenerationHeuristicLimitedDiscrepancySearchOutput& algorithm_end(Info& info);

    std::vector<double> solution;
    std::vector<std::vector<double>> x;
};

ColumnGenerationHeuristicLimitedDiscrepancySearchOutput columngenerationheuristic_limiteddiscrepancysearch(
        const Instance& instance,
        ColumnGenerationOptionalParameters parameters = {});

}

