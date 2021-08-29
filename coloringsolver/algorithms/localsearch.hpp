#pragma once

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

struct LocalSearchOptionalParameters
{
    Counter thread_number = 3;
    Counter iteration_limit = -1;
    Counter iteration_without_improvment_limit = -1;
    optimizationtools::Info info = optimizationtools::Info();
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

