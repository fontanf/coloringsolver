#pragma once

#if CPLEX_FOUND

#include "coloringsolver/solution.hpp"

namespace coloringsolver
{

/************************* Assignment-based ILP model *************************/

struct BranchAndCutAssignmentCplexOptionalParameters
{
    Info info = Info();

    const Solution* initial_solution = NULL;
    bool break_symmetries = true;
};

struct BranchAndCutAssignmentCplexOutput: Output
{
    BranchAndCutAssignmentCplexOutput(const Instance& instance, Info& info): Output(instance, info) { }
    BranchAndCutAssignmentCplexOutput& algorithm_end(Info& info);
};

BranchAndCutAssignmentCplexOutput branchandcut_assignment_cplex(
        const Instance& instance, BranchAndCutAssignmentCplexOptionalParameters parameters = {});

/************************* Representatives ILP model **************************/

struct BranchAndCutRepresentativesCplexOptionalParameters
{
    Info info = Info();

    const Solution* initial_solution = NULL;
    bool break_symmetries = true;
};

struct BranchAndCutRepresentativesCplexOutput: Output
{
    BranchAndCutRepresentativesCplexOutput(const Instance& instance, Info& info): Output(instance, info) { }
    BranchAndCutRepresentativesCplexOutput& algorithm_end(Info& info);
};

BranchAndCutRepresentativesCplexOutput branchandcut_representatives_cplex(
        const Instance& instance, BranchAndCutRepresentativesCplexOptionalParameters parameters = {});

/********************** Partial-ordering based ILP model **********************/

struct BranchAndCutPartialOrderingCplexOptionalParameters
{
    Info info = Info();

    const Solution* initial_solution = NULL;
};

struct BranchAndCutPartialOrderingCplexOutput: Output
{
    BranchAndCutPartialOrderingCplexOutput(const Instance& instance, Info& info): Output(instance, info) { }
    BranchAndCutPartialOrderingCplexOutput& algorithm_end(Info& info);
};

BranchAndCutPartialOrderingCplexOutput branchandcut_partialordering_cplex(
        const Instance& instance, BranchAndCutPartialOrderingCplexOptionalParameters parameters = {});

/********************* Partial-ordering based ILP model 2 *********************/

struct BranchAndCutPartialOrdering2CplexOptionalParameters
{
    Info info = Info();

    const Solution* initial_solution = NULL;
};

struct BranchAndCutPartialOrdering2CplexOutput: Output
{
    BranchAndCutPartialOrdering2CplexOutput(const Instance& instance, Info& info): Output(instance, info) { }
    BranchAndCutPartialOrdering2CplexOutput& algorithm_end(Info& info);
};

BranchAndCutPartialOrdering2CplexOutput branchandcut_partialordering2_cplex(
        const Instance& instance, BranchAndCutPartialOrdering2CplexOptionalParameters parameters = {});

}

#endif

