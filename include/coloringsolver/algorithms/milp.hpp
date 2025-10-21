#pragma once

#include "coloringsolver/solution.hpp"

#include "mathoptsolverscmake/milp.hpp"

namespace coloringsolver
{

struct MilpParameters: Parameters
{
    mathoptsolverscmake::SolverName solver = mathoptsolverscmake::SolverName::Highs;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Parameters::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Solver: " << solver << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters::to_json();
        json.merge_patch({
                {"Solver", solver},
                });
        return json;
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Assignment-based ILP model //////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpAssignmentParameters: MilpParameters
{
    /** Break symmetries. */
    bool break_symmetries = true;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        MilpParameters::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Break symmetries: " << break_symmetries << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = MilpParameters::to_json();
        json.merge_patch({
                {"BreakSymmetries", break_symmetries},
                });
        return json;
    }
};

Output milp_assignment(
        const Instance& instance,
        const MilpAssignmentParameters& parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Representatives ILP model ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

Output milp_representatives(
        const Instance& instance,
        const MilpParameters& parameters = {});

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Partial-ordering based ILP model ///////////////////////
////////////////////////////////////////////////////////////////////////////////

struct MilpPartialOrderingParameters: MilpParameters
{
    /** Break symmetries. */
    bool hybrid = false;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        MilpParameters::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Hybrid: " << hybrid << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = MilpParameters::to_json();
        json.merge_patch({
                {"Hybrid", hybrid},
                });
        return json;
    }
};

Output milp_partial_ordering(
        const Instance& instance,
        const MilpPartialOrderingParameters& parameters = {});

}
