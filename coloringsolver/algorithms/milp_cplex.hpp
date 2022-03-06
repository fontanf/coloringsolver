#pragma once

#if CPLEX_FOUND

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Assignment-based ILP model //////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpAssignmentCplexOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    const Solution* initial_solution = NULL;
    bool break_symmetries = true;
};

struct MilpAssignmentCplexOutput: Output
{
    MilpAssignmentCplexOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    MilpAssignmentCplexOutput& algorithm_end(
            optimizationtools::Info& info);
};

MilpAssignmentCplexOutput milp_assignment_cplex(
        const Instance& instance,
        MilpAssignmentCplexOptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Representatives ILP model ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpRepresentativesCplexOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    const Solution* initial_solution = NULL;
};

struct MilpRepresentativesCplexOutput: Output
{
    MilpRepresentativesCplexOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    MilpRepresentativesCplexOutput& algorithm_end(
            optimizationtools::Info& info);
};

MilpRepresentativesCplexOutput milp_representatives_cplex(
        const Instance& instance,
        MilpRepresentativesCplexOptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Partial-ordering based ILP model ///////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpPartialOrderingCplexOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    const Solution* initial_solution = NULL;
};

struct MilpPartialOrderingCplexOutput: Output
{
    MilpPartialOrderingCplexOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    MilpPartialOrderingCplexOutput& algorithm_end(
            optimizationtools::Info& info);
};

MilpPartialOrderingCplexOutput milp_partialordering_cplex(
        const Instance& instance,
        MilpPartialOrderingCplexOptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////// Partial-ordering based ILP model 2 //////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpPartialOrdering2CplexOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    const Solution* initial_solution = NULL;
};

struct MilpPartialOrdering2CplexOutput: Output
{
    MilpPartialOrdering2CplexOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    MilpPartialOrdering2CplexOutput& algorithm_end(
            optimizationtools::Info& info);
};

MilpPartialOrdering2CplexOutput milp_partialordering2_cplex(
        const Instance& instance,
        MilpPartialOrdering2CplexOptionalParameters parameters = {});

}

#endif

