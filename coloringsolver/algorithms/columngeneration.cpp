#include "coloringsolver/algorithms/columngeneration.hpp"

using namespace coloringsolver;

/**
 * The linear programming formulation of the Graph Coloring Problem based on
 * Dantzig–Wolfe decomposition is written as follows:
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

ColumnGenerationHeuristicGreedyOutput& ColumnGenerationHeuristicGreedyOutput::algorithm_end(Info& info)
{
    Output::algorithm_end(info);
    return *this;
}

ColumnGenerationHeuristicLimitedDiscrepancySearchOutput& ColumnGenerationHeuristicLimitedDiscrepancySearchOutput::algorithm_end(Info& info)
{
    Output::algorithm_end(info);
    return *this;
}

class PricingSolver: public columngenerationsolver::PricingSolver
{

public:

    PricingSolver(const Instance& instance):
        instance_(instance),
        fixed_vertices_(instance.vertex_number()),
        coloring2mwis_(instance.vertex_number())
    {  }

    virtual void initialize_pricing(
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

columngenerationsolver::Parameters get_parameters(
        const Instance& instance,
        columngenerationsolver::LinearProgrammingSolver linear_programming_solver)
{
    VertexId n = instance.vertex_number();
    columngenerationsolver::Parameters p(n);

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
    p.dummy_column_objective_coefficient = instance.degree_max();
    // Pricing solver.
    p.pricing_solver = std::unique_ptr<columngenerationsolver::PricingSolver>(
            new PricingSolver(instance));
    p.linear_programming_solver = linear_programming_solver;
    return p;
}

Solution columns2solution(
        const Instance& instance,
        const std::vector<Column>& columns,
        const std::vector<std::pair<ColIdx, Value>>& column_solution)
{
    Solution solution(instance);
    ColorId color = 0;
    for (auto pair: column_solution) {
        ColIdx col_idx = pair.first;
        Value value = pair.second;
        if (value < 0.5)
            continue;
        const Column& column = columns[col_idx];
        for (RowIdx row_pos = 0; row_pos < (RowIdx)column.row_indices.size(); ++row_pos)
            if (column.row_coefficients[row_pos] > 0.5)
                solution.set(column.row_indices[row_pos], color);
        color++;
    }
    return solution;
}

void PricingSolver::initialize_pricing(
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
}

std::vector<Column> PricingSolver::solve_pricing(
            const std::vector<Value>& duals)
{
    VertexId n = instance_.vertex_number();
    std::vector<Column> columns;
    stablesolver::Weight mult = 10000;

    // Build Maximum-Weight Independent Set instance.
    std::fill(coloring2mwis_.begin(), coloring2mwis_.end(), -1);
    mwis2coloring_.clear();
    weights_.clear();
    for (VertexId v = 0; v < n; ++v) {
        if (fixed_vertices_[v] == 1)
            continue;
        stablesolver::Weight weight = std::floor(mult * duals[v]);
        if (weight <= 0)
            continue;
        coloring2mwis_[v] = mwis2coloring_.size();
        mwis2coloring_.push_back(v);
        weights_.push_back(weight);
    }
    stablesolver::Instance instance_mwis(mwis2coloring_.size());
    for (stablesolver::VertexId v1 = 0; v1 < instance_mwis.vertex_number(); ++v1) {
        // Set weight.
        instance_mwis.set_weight(v1, weights_[v1]);
        // Add edges.
        VertexId v1_orig = mwis2coloring_[v1];
        for (const VertexNeighbor& vn: instance_.vertex(v1_orig).edges) {
            stablesolver::VertexId v2 = coloring2mwis_[vn.v];
            if (v2 != -1 && v2 > v1)
                instance_mwis.add_edge(v1, v2);
        }
    }
    instance_mwis.compute_components();

    // Solve Maximum-Weight Independent Set instance.
    stablesolver::LargeNeighborhoodSearchOptionalParameters parameters_mwis;
    parameters_mwis.iteration_without_improvment_limit = 1000;
    //parameters_mwis.info.set_verbose(true);
    auto output_mwis = stablesolver::largeneighborhoodsearch(instance_mwis, parameters_mwis);

    // Retrieve column.
    Column column;
    for (stablesolver::VertexId v = 0; v < instance_mwis.vertex_number(); ++v) {
        if (output_mwis.solution.contains(v)) {
            column.row_indices.push_back(mwis2coloring_[v]);
            column.row_coefficients.push_back(1);
        }
    }
    column.objective_coefficient = 1;
    return {column};
}

/******************************************************************************/

ColumnGenerationHeuristicGreedyOutput coloringsolver::columngenerationheuristic_greedy(
        const Instance& instance, ColumnGenerationOptionalParameters parameters)
{
    VER(parameters.info, "*** columngenerationheuristic_greedy"
            << " --linear-programming-solver " << parameters.linear_programming_solver
            << " ***" << std::endl);
    ColumnGenerationHeuristicGreedyOutput output(instance, parameters.info);

    columngenerationsolver::Parameters p = get_parameters(
            instance, parameters.linear_programming_solver);
    columngenerationsolver::GreedyOptionalParameters op;
    op.info.set_timelimit(parameters.info.remaining_time());
    // Self-ajusting Wentges smoothing and automatic directional smoothing do
    // not work well on the Graph Coloring Problem as shown in "Automation and
    // Combination of Linear-Programming Based Stabilization Techniques in
    // Column Generation" (Pessoa et al., 2018). Therefore, let us disable
    // these options.
    op.columngeneration_parameters.static_wentges_smoothing_parameter = 0.0;
    op.columngeneration_parameters.static_directional_smoothing_parameter = 0.0;
    op.columngeneration_parameters.self_adjusting_wentges_smoothing = false;
    op.columngeneration_parameters.automatic_directional_smoothing = false;
    auto output_greedy = columngenerationsolver::greedy(p, op);

    //output.update_lower_bound(
    //        std::ceil(output_greedy.bound - TOL),
    //        std::stringstream(""),
    //        parameters.info);
    if (output_greedy.solution.size() > 0)
        output.update_solution(
                columns2solution(instance, p.columns, output_greedy.solution),
                std::stringstream(""),
                parameters.info);
    return output.algorithm_end(parameters.info);
}

ColumnGenerationHeuristicLimitedDiscrepancySearchOutput coloringsolver::columngenerationheuristic_limiteddiscrepancysearch(
        const Instance& instance, ColumnGenerationOptionalParameters parameters)
{
    VER(parameters.info, "*** columngenerationheuristic_limiteddiscrepancysearch"
            << " --linear-programming-solver " << parameters.linear_programming_solver
            << " ***" << std::endl);
    ColumnGenerationHeuristicLimitedDiscrepancySearchOutput output(instance, parameters.info);

    columngenerationsolver::Parameters p = get_parameters(
            instance, parameters.linear_programming_solver);
    columngenerationsolver::LimitedDiscrepancySearchOptionalParameters op;
    op.new_bound_callback = [&instance, &parameters, &p, &output](
                const columngenerationsolver::LimitedDiscrepancySearchOutput& o)
        {
            std::stringstream ss;
            ss << "node " << o.node_number;
            if (o.solution.size() > 0) {
                ss << " discrepancy " << o.solution_discrepancy;
                output.update_solution(
                        columns2solution(instance, p.columns, o.solution),
                        ss,
                        parameters.info);
            }
            //output.update_lower_bound(
            //        std::ceil(o.bound - TOL),
            //        ss,
            //        parameters.info);
        };
    op.columngeneration_parameters.static_wentges_smoothing_parameter = 0.0;
    op.columngeneration_parameters.static_directional_smoothing_parameter = 0.0;
    op.columngeneration_parameters.self_adjusting_wentges_smoothing = false;
    op.columngeneration_parameters.automatic_directional_smoothing = false;
    op.info.set_timelimit(parameters.info.remaining_time());

    auto output_limiteddiscrepancysearch = columngenerationsolver::limiteddiscrepancysearch( p, op);
    return output.algorithm_end(parameters.info);
}
