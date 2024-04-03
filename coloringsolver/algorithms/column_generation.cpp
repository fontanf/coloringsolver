#include "coloringsolver/algorithms/column_generation.hpp"

#include "coloringsolver/algorithm_formatter.hpp"

#include "columngenerationsolver/algorithms/column_generation.hpp"
#include "columngenerationsolver/algorithms/greedy.hpp"
#include "columngenerationsolver/algorithms/limited_discrepancy_search.hpp"
#include "columngenerationsolver/algorithms/heuristic_tree_search.hpp"

#include "stablesolver/stable/instance_builder.hpp"
#include "stablesolver/stable/algorithms/local_search.hpp"

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

    virtual std::vector<std::shared_ptr<const Column>> initialize_pricing(
            const std::vector<std::pair<std::shared_ptr<const Column>, Value>>& fixed_columns);

    virtual std::vector<std::shared_ptr<const Column>> solve_pricing(
            const std::vector<Value>& duals);

private:

    const Instance& instance_;

    std::vector<int8_t> fixed_vertices_;

    std::vector<VertexId> mwis2coloring_;

    std::vector<stablesolver::stable::VertexId> coloring2mwis_;

    std::vector<stablesolver::stable::Weight> weights_;

};

columngenerationsolver::Model get_model(const Instance& instance)
{
    columngenerationsolver::Model model;

    model.objective_sense = optimizationtools::ObjectiveDirection::Minimize;
    model.column_lower_bound = 0;
    model.column_upper_bound = 1;

    for (VertexId vertex_id = 0;
            vertex_id < instance.graph().number_of_vertices();
            ++vertex_id) {
        columngenerationsolver::Row row;
        row.lower_bound = 1;
        row.upper_bound = 1;
        row.coefficient_lower_bound = 0;
        row.coefficient_upper_bound = 1;
        model.rows.push_back(row);
    }

    // Pricing solver.
    model.pricing_solver = std::unique_ptr<columngenerationsolver::PricingSolver>(
            new PricingSolver(instance));

    return model;
}

Solution columns2solution(
        const Instance& instance,
        const std::vector<std::pair<std::shared_ptr<const Column>, Value>>& columns)
{
    Solution solution(instance);
    ColorId color = 0;
    for (auto pair: columns) {
        Value value = pair.second;
        if (value < 0.5)
            continue;
        const Column& column = *(pair.first);
        for (const columngenerationsolver::LinearTerm& element: column.elements) {
            if (element.coefficient > 0.5)
                solution.set(element.row, color);
        }
        color++;
    }
    return solution;
}

std::vector<std::shared_ptr<const Column>> PricingSolver::initialize_pricing(
            const std::vector<std::pair<std::shared_ptr<const Column>, Value>>& fixed_columns)
{
    std::fill(fixed_vertices_.begin(), fixed_vertices_.end(), -1);
    for (auto p: fixed_columns) {
        const Column& column = *(p.first);
        Value value = p.second;
        if (value < 0.5)
            continue;
        for (const columngenerationsolver::LinearTerm& element: column.elements) {
            if (element.coefficient < 0.5)
                continue;
            fixed_vertices_[element.row] = 1;
        }
    }
    return {};
}

std::vector<std::shared_ptr<const Column>> PricingSolver::solve_pricing(
            const std::vector<Value>& duals)
{
    std::vector<std::shared_ptr<const Column>> columns;
    stablesolver::stable::Weight mult = 10000;

    // Build subproblem instance.
    std::fill(coloring2mwis_.begin(), coloring2mwis_.end(), -1);
    mwis2coloring_.clear();
    stablesolver::stable::InstanceBuilder mwis_instance_builder;
    // Add vertices.
    for (VertexId vertex_id = 0;
            vertex_id < instance_.graph().number_of_vertices();
            ++vertex_id) {
        if (fixed_vertices_[vertex_id] == 1)
            continue;
        stablesolver::stable::Weight weight = std::floor(mult * duals[vertex_id]);
        if (weight <= 0)
            continue;
        coloring2mwis_[vertex_id] = mwis2coloring_.size();
        mwis2coloring_.push_back(vertex_id);
        mwis_instance_builder.add_vertex(weight);
    }
    // Add edges.
    for (stablesolver::stable::VertexId mwis_vertex_id_1 = 0;
            mwis_vertex_id_1 < (stablesolver::stable::VertexId)mwis2coloring_.size();
            ++mwis_vertex_id_1) {
        VertexId vertex_id_1 = mwis2coloring_[mwis_vertex_id_1];
        for (auto it = instance_.graph().neighbors_begin(vertex_id_1);
                it != instance_.graph().neighbors_end(vertex_id_1); ++it) {
            VertexId vertex_id_2 = *it;
            stablesolver::stable::VertexId mwis_vertex_id_2 = coloring2mwis_[vertex_id_2];
            if (mwis_vertex_id_2 != -1
                    && mwis_vertex_id_2 > mwis_vertex_id_1) {
                mwis_instance_builder.add_edge(
                        mwis_vertex_id_1,
                        mwis_vertex_id_2);
            }
        }
    }
    stablesolver::stable::Instance mwis_instance = mwis_instance_builder.build();

    // Solve subproblem instance.
    stablesolver::stable::LocalSearchParameters mwis_parameters;
    mwis_parameters.maximum_number_of_nodes = 1000;
    mwis_parameters.verbosity_level = 0;
    auto output_mwis = stablesolver::stable::local_search(
            mwis_instance,
            mwis_parameters);

    // Retrieve column.
    Column column;
    for (stablesolver::stable::VertexId mwis_vertex_id = 0;
            mwis_vertex_id < mwis_instance.number_of_vertices();
            ++mwis_vertex_id) {
        if (output_mwis.solution.contains(mwis_vertex_id)) {
            columngenerationsolver::LinearTerm element;
            element.row = mwis2coloring_[mwis_vertex_id];
            element.coefficient = 1;
            column.elements.push_back(element);
        }
    }
    column.objective_coefficient = 1;

    columns.push_back(std::shared_ptr<const Column>(new Column(column)));
    return columns;
}

const Output coloringsolver::column_generation_heuristic_greedy(
        const Instance& instance,
        const ColumnGenerationParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Column generation heuristic - Greedy");
    algorithm_formatter.print_header();

    columngenerationsolver::Model model = get_model(instance);
    columngenerationsolver::GreedyParameters cgsg_parameters;
    cgsg_parameters.verbosity_level = 0;
    cgsg_parameters.timer = parameters.timer;
    cgsg_parameters.column_generation_parameters.linear_programming_solver
        = columngenerationsolver::s2lps(parameters.linear_programming_solver);
    // Self-ajusting Wentges smoothing and automatic directional smoothing do
    // not work well on the Graph Coloring Problem as shown in "Automation and
    // Combination of Linear-Programming Based Stabilization Techniques in
    // Column Generation" (Pessoa et al., 2018). Therefore, let us disable
    // these options.
    cgsg_parameters.column_generation_parameters.static_wentges_smoothing_parameter = 0.0;
    cgsg_parameters.column_generation_parameters.static_directional_smoothing_parameter = 0.0;
    cgsg_parameters.column_generation_parameters.self_adjusting_wentges_smoothing = false;
    cgsg_parameters.column_generation_parameters.automatic_directional_smoothing = false;
    auto output_greedy = columngenerationsolver::greedy(model, cgsg_parameters);

    //algorithm_formatter.update_lower_bound(
    //        std::ceil(output_greedy.bound - FFOT_TOL),
    //        "");
    if (output_greedy.solution.feasible()) {
        algorithm_formatter.update_solution(
                columns2solution(instance, output_greedy.solution.columns()),
                "");
    }

    algorithm_formatter.end();
    return output;
}

const Output coloringsolver::column_generation_heuristic_limited_discrepancy_search(
        const Instance& instance,
        const ColumnGenerationParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Column generation heuristic - Limited discrepancy search");
    algorithm_formatter.print_header();

    columngenerationsolver::Model model = get_model(instance);
    columngenerationsolver::LimitedDiscrepancySearchParameters cgslds_parameters;
    cgslds_parameters.verbosity_level = 1;
    cgslds_parameters.timer = parameters.timer;
    cgslds_parameters.internal_diving = 1;
    cgslds_parameters.column_generation_parameters.linear_programming_solver
        = columngenerationsolver::s2lps(parameters.linear_programming_solver);
    cgslds_parameters.new_solution_callback = [&instance, &algorithm_formatter](
                const columngenerationsolver::Output& cgs_output)
        {
            const columngenerationsolver::LimitedDiscrepancySearchOutput& cgslds_output
                = static_cast<const columngenerationsolver::LimitedDiscrepancySearchOutput&>(cgs_output);
            std::stringstream ss;
            ss << "node " << cgslds_output.number_of_nodes;
            if (cgslds_output.solution.feasible()) {
                ss << " discrepancy " << cgslds_output.maximum_discrepancy;
                algorithm_formatter.update_solution(
                        columns2solution(instance, cgslds_output.solution.columns()),
                        ss.str());
            }
            //algorithm_formatter.update_lower_bound(
            //        std::ceil(o.bound - FFOT_TOL),
            //        ss.str());
        };
    cgslds_parameters.column_generation_parameters.static_wentges_smoothing_parameter = 0.0;
    cgslds_parameters.column_generation_parameters.static_directional_smoothing_parameter = 0.0;
    cgslds_parameters.column_generation_parameters.self_adjusting_wentges_smoothing = false;
    cgslds_parameters.column_generation_parameters.automatic_directional_smoothing = false;

    auto output_limiteddiscrepancysearch = columngenerationsolver::limited_discrepancy_search(model, cgslds_parameters);

    algorithm_formatter.end();
    return output;
}

const Output coloringsolver::column_generation_heuristic_heuristic_tree_search(
        const Instance& instance,
        const ColumnGenerationParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Column generation heuristic - Heuristic tree search");
    algorithm_formatter.print_header();

    columngenerationsolver::Model model = get_model(instance);
    columngenerationsolver::HeuristicTreeSearchParameters cgshts_parameters;
    cgshts_parameters.column_generation_parameters.linear_programming_solver
        = columngenerationsolver::s2lps(parameters.linear_programming_solver);
    cgshts_parameters.new_solution_callback = [&instance, &algorithm_formatter](
                const columngenerationsolver::Output& cgs_output)
        {
            const columngenerationsolver::HeuristicTreeSearchOutput& cgshts_output
                = static_cast<const columngenerationsolver::HeuristicTreeSearchOutput&>(cgs_output);
            if (cgshts_output.solution.feasible()) {
                std::stringstream ss;
                ss << "it " << cgshts_output.maximum_number_of_iterations;
                algorithm_formatter.update_solution(
                        columns2solution(instance, cgshts_output.solution.columns()),
                        ss.str());
            }
        };
    cgshts_parameters.column_generation_parameters.static_wentges_smoothing_parameter = 0.0;
    cgshts_parameters.column_generation_parameters.static_directional_smoothing_parameter = 0.0;
    cgshts_parameters.column_generation_parameters.self_adjusting_wentges_smoothing = false;
    cgshts_parameters.column_generation_parameters.automatic_directional_smoothing = false;

    auto output_heuristictreesearch = columngenerationsolver::heuristic_tree_search(model, cgshts_parameters);

    algorithm_formatter.end();
    return output;
}
