#include "coloringsolver/algorithms/columngeneration.hpp"

#include "columngenerationsolver/algorithms/column_generation.hpp"
#include "columngenerationsolver/algorithms/greedy.hpp"
#include "columngenerationsolver/algorithms/limited_discrepancy_search.hpp"
#include "columngenerationsolver/algorithms/heuristic_tree_search.hpp"

#include "stablesolver/algorithms/localsearch.hpp"

/**
 * The linear programming formulation of the problem based on Dantzig–Wolfe
 * decomposition is written as follows:
 *
 * Variables:
 * - yᵏ ∈ {0, 1} representing a stable set.
 *   yᵏ = 1 iff the corresponding set of items is assigned to agent i.
 *   xᵥᵏ = 1 iff yᵏ contains vertex v, otherwise 0.
 *
 * Program:
 *
 * min ∑ₖ yᵏ
 *
 * 1 <= ∑ₖ xᵥᵏ yᵏ <= 1  for all vertex v
 *                                         (each vertex colored exactly once)
 *                                                         Dual variables: vᵥ
 *
 * The pricing problem consists in finding a variable of negative reduced cost.
 * The reduced cost of a variable yᵏ is given by:
 * rc(yᵢᵏ) = 1 - ∑ᵥ xᵥᵏ vᵥ
 *
 * Therefore, finding a variable of minium reduced cost reduces to solving
 * a Maximum-Weight Independent Set Problem with vertices with weight vᵥ.
 *
 */

using namespace coloringsolver;

using RowIdx = columngenerationsolver::RowIdx;
using ColIdx = columngenerationsolver::ColIdx;
using Value = columngenerationsolver::Value;
using Column = columngenerationsolver::Column;

class PricingSolver: public columngenerationsolver::PricingSolver
{

public:

    PricingSolver(const Instance& instance):
        instance_(instance),
        fixed_vertices_(instance.graph().number_of_vertices()),
        coloring2mwis_(instance.graph().number_of_vertices())
    {  }

    virtual std::vector<ColIdx> initialize_pricing(
            const std::vector<Column>& columns,
            const std::vector<std::pair<ColIdx, Value>>& fixed_columns);

    virtual std::vector<Column> solve_pricing(
            const std::vector<Value>& duals);

private:

    const Instance& instance_;

    std::vector<int8_t> fixed_vertices_;

    std::vector<VertexId> mwis2coloring_;
    std::vector<stablesolver::VertexId> coloring2mwis_;
    std::vector<stablesolver::Weight> weights_;

};

columngenerationsolver::Parameters get_parameters(const Instance& instance)
{
    columngenerationsolver::Parameters p(instance.graph().number_of_vertices());

    p.objective_sense = columngenerationsolver::ObjectiveSense::Min;
    p.column_lower_bound = 0;
    p.column_upper_bound = 1;
    // Row lower bounds.
    std::fill(p.row_lower_bounds.begin(), p.row_lower_bounds.end(), 1);
    // Row upper bounds.
    std::fill(p.row_upper_bounds.begin(), p.row_upper_bounds.end(), 1);
    // Row coefficent lower bounds.
    std::fill(p.row_coefficient_lower_bounds.begin(), p.row_coefficient_lower_bounds.end(), 0);
    // Row coefficent upper bounds.
    std::fill(p.row_coefficient_upper_bounds.begin(), p.row_coefficient_upper_bounds.end(), 1);
    // Dummy column objective coefficient.
    p.dummy_column_objective_coefficient = instance.graph().maximum_degree();
    // Pricing solver.
    p.pricing_solver = std::unique_ptr<columngenerationsolver::PricingSolver>(
            new PricingSolver(instance));
    return p;
}

Solution columns2solution(
        const Instance& instance,
        const std::vector<std::pair<Column, Value>>& columns)
{
    Solution solution(instance);
    ColorId color = 0;
    for (auto pair: columns) {
        Value value = pair.second;
        if (value < 0.5)
            continue;
        const Column& column = pair.first;
        for (RowIdx row_pos = 0; row_pos < (RowIdx)column.row_indices.size(); ++row_pos)
            if (column.row_coefficients[row_pos] > 0.5)
                solution.set(column.row_indices[row_pos], color);
        color++;
    }
    return solution;
}

std::vector<ColIdx> PricingSolver::initialize_pricing(
            const std::vector<Column>& columns,
            const std::vector<std::pair<ColIdx, Value>>& fixed_columns)
{
    std::fill(fixed_vertices_.begin(), fixed_vertices_.end(), -1);
    for (auto p: fixed_columns) {
        const Column& column = columns[p.first];
        Value value = p.second;
        if (value < 0.5)
            continue;
        for (RowIdx row_pos = 0; row_pos < (RowIdx)column.row_indices.size(); ++row_pos) {
            RowIdx row_index = column.row_indices[row_pos];
            Value row_coefficient = column.row_coefficients[row_pos];
            if (row_coefficient < 0.5)
                continue;
            fixed_vertices_[row_index] = 1;
        }
    }
    return {};
}

std::vector<Column> PricingSolver::solve_pricing(
            const std::vector<Value>& duals)
{
    VertexId n = instance_.graph().number_of_vertices();
    std::vector<Column> columns;
    stablesolver::Weight mult = 10000;

    // Build subproblem instance.
    std::fill(coloring2mwis_.begin(), coloring2mwis_.end(), -1);
    mwis2coloring_.clear();
    weights_.clear();
    for (VertexId vertex_id = 0; vertex_id < n; ++vertex_id) {
        if (fixed_vertices_[vertex_id] == 1)
            continue;
        stablesolver::Weight weight = std::floor(mult * duals[vertex_id]);
        if (weight <= 0)
            continue;
        coloring2mwis_[vertex_id] = mwis2coloring_.size();
        mwis2coloring_.push_back(vertex_id);
        weights_.push_back(weight);
    }
    stablesolver::Instance instance_mwis(mwis2coloring_.size());
    for (stablesolver::VertexId mwis_vertex_id_1 = 0;
            mwis_vertex_id_1 < instance_mwis.number_of_vertices();
            ++mwis_vertex_id_1) {
        // Set weight.
        instance_mwis.set_weight(
                mwis_vertex_id_1,
                weights_[mwis_vertex_id_1]);
        // Add edges.
        VertexId vertex_id_1 = mwis2coloring_[mwis_vertex_id_1];
        for (auto it = instance_.graph().neighbors_begin(vertex_id_1);
                it != instance_.graph().neighbors_end(vertex_id_1); ++it) {
            VertexId vertex_id_2 = *it;
            stablesolver::VertexId mwis_vertex_id_2 = coloring2mwis_[vertex_id_2];
            if (mwis_vertex_id_2 != -1
                    && mwis_vertex_id_2 > mwis_vertex_id_1) {
                instance_mwis.add_edge(
                        mwis_vertex_id_1,
                        mwis_vertex_id_2);
            }
        }
    }
    instance_mwis.compute_components();

    // Solve subproblem instance.
    stablesolver::LocalSearchOptionalParameters parameters_mwis;
    parameters_mwis.maximum_number_of_nodes = 1000;
    //parameters_mwis.info.set_verbose(true);
    std::mt19937_64 generator(0);
    auto output_mwis = stablesolver::localsearch(instance_mwis, generator, parameters_mwis);

    // Retrieve column.
    Column column;
    for (stablesolver::VertexId mwis_vertex_id = 0;
            mwis_vertex_id < instance_mwis.number_of_vertices();
            ++mwis_vertex_id) {
        if (output_mwis.solution.contains(mwis_vertex_id)) {
            column.row_indices.push_back(mwis2coloring_[mwis_vertex_id]);
            column.row_coefficients.push_back(1);
        }
    }
    column.objective_coefficient = 1;
    return {column};
}

Output coloringsolver::columngenerationheuristic_greedy(
        const Instance& instance, ColumnGenerationOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Column generation heuristic - Greedy" << std::endl
        << std::endl;
    Output output(instance, parameters.info);

    columngenerationsolver::Parameters p = get_parameters(instance);
    columngenerationsolver::GreedyOptionalParameters op;
    op.info.set_time_limit(parameters.info.remaining_time());
    op.column_generation_parameters.linear_programming_solver
        = columngenerationsolver::s2lps(parameters.linear_programming_solver);
    // Self-ajusting Wentges smoothing and automatic directional smoothing do
    // not work well on the Graph Coloring Problem as shown in "Automation and
    // Combination of Linear-Programming Based Stabilization Techniques in
    // Column Generation" (Pessoa et al., 2018). Therefore, let us disable
    // these options.
    op.column_generation_parameters.static_wentges_smoothing_parameter = 0.0;
    op.column_generation_parameters.static_directional_smoothing_parameter = 0.0;
    op.column_generation_parameters.self_adjusting_wentges_smoothing = false;
    op.column_generation_parameters.automatic_directional_smoothing = false;
    //op.columngeneration_parameters.info.set_verbose(true);
    auto output_greedy = columngenerationsolver::greedy(p, op);

    //output.update_lower_bound(
    //        std::ceil(output_greedy.bound - FFOT_TOL),
    //        std::stringstream(""),
    //        parameters.info);
    if (output_greedy.solution.size() > 0)
        output.update_solution(
                columns2solution(instance, output_greedy.solution),
                std::stringstream(""),
                parameters.info);
    return output.algorithm_end(parameters.info);
}

Output coloringsolver::columngenerationheuristic_limiteddiscrepancysearch(
        const Instance& instance, ColumnGenerationOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Column generation heuristic - Limited discrepancy search" << std::endl
        << std::endl;
    Output output(instance, parameters.info);

    columngenerationsolver::Parameters p = get_parameters(instance);
    columngenerationsolver::LimitedDiscrepancySearchOptionalParameters op;
    op.column_generation_parameters.linear_programming_solver
        = columngenerationsolver::s2lps(parameters.linear_programming_solver);
    op.new_bound_callback = [&instance, &parameters, &output](
                const columngenerationsolver::LimitedDiscrepancySearchOutput& o)
        {
            std::stringstream ss;
            ss << "node " << o.number_of_nodes;
            if (o.solution.size() > 0) {
                ss << " discrepancy " << o.solution_discrepancy;
                output.update_solution(
                        columns2solution(instance, o.solution),
                        ss,
                        parameters.info);
            }
            //output.update_lower_bound(
            //        std::ceil(o.bound - FFOT_TOL),
            //        ss,
            //        parameters.info);
        };
    op.column_generation_parameters.static_wentges_smoothing_parameter = 0.0;
    op.column_generation_parameters.static_directional_smoothing_parameter = 0.0;
    op.column_generation_parameters.self_adjusting_wentges_smoothing = false;
    op.column_generation_parameters.automatic_directional_smoothing = false;
    //op.columngeneration_parameters.info.set_verbose(true);
    op.info.set_time_limit(parameters.info.remaining_time());

    auto output_limiteddiscrepancysearch = columngenerationsolver::limited_discrepancy_search( p, op);
    return output.algorithm_end(parameters.info);
}

Output coloringsolver::columngenerationheuristic_heuristictreesearch(
        const Instance& instance, ColumnGenerationOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Column generation heuristic - Heuristic tree search" << std::endl
        << std::endl;
    Output output(instance, parameters.info);

    columngenerationsolver::Parameters p = get_parameters(instance);
    columngenerationsolver::HeuristicTreeSearchOptionalParameters op;
    op.column_generation_parameters.linear_programming_solver
        = columngenerationsolver::s2lps(parameters.linear_programming_solver);
    op.new_bound_callback = [&instance, &parameters, &output](
                const columngenerationsolver::HeuristicTreeSearchOutput& o)
        {
            if (o.solution.size() > 0) {
                std::stringstream ss;
                ss << "it " << o.maximum_number_of_iterations;
                output.update_solution(
                        columns2solution(instance, o.solution),
                        ss,
                        parameters.info);
            }
        };
    op.column_generation_parameters.static_wentges_smoothing_parameter = 0.0;
    op.column_generation_parameters.static_directional_smoothing_parameter = 0.0;
    op.column_generation_parameters.self_adjusting_wentges_smoothing = false;
    op.column_generation_parameters.automatic_directional_smoothing = false;
    //op.info.set_verbose(true);
    //op.columngeneration_parameters.info.set_verbose(true);
    op.info.set_time_limit(parameters.info.remaining_time());

    auto output_heuristictreesearch = columngenerationsolver::heuristic_tree_search( p, op);
    return output.algorithm_end(parameters.info);
}
