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
    /** Initial solution. */
    const Solution* initial_solution = NULL;

    /** Break symmetries. */
    bool break_symmetries = true;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

Output milp_assignment_cplex(
        const Instance& instance,
        MilpAssignmentCplexOptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Representatives ILP model ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpRepresentativesCplexOptionalParameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

Output milp_representatives_cplex(
        const Instance& instance,
        MilpRepresentativesCplexOptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Partial-ordering based ILP model ///////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpPartialOrderingCplexOptionalParameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

Output milp_partialordering_cplex(
        const Instance& instance,
        MilpPartialOrderingCplexOptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////// Partial-ordering based ILP model 2 //////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpPartialOrdering2CplexOptionalParameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

Output milp_partialordering2_cplex(
        const Instance& instance,
        MilpPartialOrdering2CplexOptionalParameters parameters = {});

}

#endif

