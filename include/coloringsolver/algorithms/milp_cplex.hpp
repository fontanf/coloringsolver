#pragma once

#if CPLEX_FOUND

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Assignment-based ILP model //////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpAssignmentCplexParameters: Parameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;

    /** Break symmetries. */
    bool break_symmetries = true;
};

const Output milp_assignment_cplex(
        const Instance& instance,
        const MilpAssignmentCplexParameters& parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Representatives ILP model ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpRepresentativesCplexParameters: Parameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;
};

const Output milp_representatives_cplex(
        const Instance& instance,
        const MilpRepresentativesCplexParameters& parameters = {});

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Partial-ordering based ILP model ///////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpPartialOrderingCplexParameters: Parameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;
};

const Output milp_partialordering_cplex(
        const Instance& instance,
        const MilpPartialOrderingCplexParameters& parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////// Partial-ordering based ILP model 2 //////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpPartialOrdering2CplexParameters: Parameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;
};

const Output milp_partialordering2_cplex(
        const Instance& instance,
        const MilpPartialOrdering2CplexParameters& parameters = {});

}

#endif
