#include "coloringsolver/algorithms/milp.hpp"

#include "coloringsolver/algorithm_formatter.hpp"

using namespace coloringsolver;

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Assignment-based ILP model //////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace
{

struct ModelAssignment
{
    /** Model. */
    mathoptsolverscmake::MilpModel model;

    /** x[v][c] = 1 iff vertex 'v' is assigned color 'c'. */
    std::vector<std::vector<int>> x;

    /** y[c] = 1 iff color 'c' is used. */
    std::vector<int> y;
};

ModelAssignment create_milp_model_assignment(
        const Instance& instance,
        const MilpAssignmentParameters& parameters)
{
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    ModelAssignment model;

    /////////////////////////////
    // Variables and objective //
    /////////////////////////////

    model.model.objective_direction = mathoptsolverscmake::ObjectiveDirection::Minimize;

    // Variables x.
    ColorId upper_bound = graph.highest_degree() + 1;
    model.x = std::vector<std::vector<int>>(graph.number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        model.x[vertex_id] = std::vector<int>(upper_bound);
        for (ColorId color_id = 0;
                color_id < upper_bound;
                ++color_id) {
            model.x[vertex_id][color_id] = model.model.variables_lower_bounds.size();
            model.model.variables_lower_bounds.push_back(0);
            model.model.variables_upper_bounds.push_back(1);
            model.model.variables_types.push_back(mathoptsolverscmake::VariableType::Binary);
            model.model.objective_coefficients.push_back(0);
        }
    }

    // Variables y.
    model.y = std::vector<int>(upper_bound, -1);
    for (ColorId color_id = 0;
            color_id < upper_bound;
            ++color_id) {
        model.y[color_id] = model.model.variables_lower_bounds.size();
        model.model.variables_lower_bounds.push_back(0);
        model.model.variables_upper_bounds.push_back(1);
        model.model.variables_types.push_back(mathoptsolverscmake::VariableType::Binary);
        model.model.objective_coefficients.push_back(1);
    }

    /////////////////
    // Constraints //
    /////////////////

    // Each vertex must have a color.
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        model.model.constraints_starts.push_back(model.model.elements_variables.size());
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
            model.model.elements_variables.push_back(model.x[vertex_id][color_id]);
            model.model.elements_coefficients.push_back(1.0);
        }
        model.model.constraints_lower_bounds.push_back(1);
        model.model.constraints_upper_bounds.push_back(1);
    }

    // 2 neighbors must have different colors.
    for (EdgeId edge_id = 0; edge_id < graph.number_of_edges(); ++edge_id) {
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
            model.model.constraints_starts.push_back(model.model.elements_variables.size());

            model.model.elements_variables.push_back(model.x[graph.first_end(edge_id)][color_id]);
            model.model.elements_coefficients.push_back(1.0);
            model.model.elements_variables.push_back(model.x[graph.second_end(edge_id)][color_id]);
            model.model.elements_coefficients.push_back(1.0);
            model.model.elements_variables.push_back(model.y[color_id]);
            model.model.elements_coefficients.push_back(-1.0);

            model.model.constraints_lower_bounds.push_back(-std::numeric_limits<double>::infinity());
            model.model.constraints_upper_bounds.push_back(0);
        }
    }
    // Handle degree 0 vertices.
    for (VertexId vertex_id = 0; vertex_id < graph.number_of_vertices(); ++vertex_id) {
        if (graph.degree(vertex_id) > 0)
            continue;
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
            model.model.constraints_starts.push_back(model.model.elements_variables.size());

            model.model.elements_variables.push_back(model.x[vertex_id][color_id]);
            model.model.elements_coefficients.push_back(1.0);
            model.model.elements_variables.push_back(model.y[color_id]);
            model.model.elements_coefficients.push_back(-1.0);

            model.model.constraints_lower_bounds.push_back(-std::numeric_limits<double>::infinity());
            model.model.constraints_upper_bounds.push_back(0);
        }
    }

    if (parameters.break_symmetries) {
        // y[c] <= \sum_v x[v][c]
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
            model.model.constraints_starts.push_back(model.model.elements_variables.size());

            model.model.elements_variables.push_back(model.y[color_id]);
            model.model.elements_coefficients.push_back(1.0);
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id) {
                model.model.elements_variables.push_back(model.x[vertex_id][color_id]);
                model.model.elements_coefficients.push_back(-1.0);
            }

            model.model.constraints_lower_bounds.push_back(-std::numeric_limits<double>::infinity());
            model.model.constraints_upper_bounds.push_back(0);
        }

        // Constraints: use color 'c' only if color 'c - 1' is used.
        // y[c] <= y[c - 1]
        for (ColorId color_id = 1; color_id < upper_bound; ++color_id) {
            model.model.constraints_starts.push_back(model.model.elements_variables.size());

            model.model.elements_variables.push_back(model.y[color_id]);
            model.model.elements_coefficients.push_back(1.0);
            model.model.elements_variables.push_back(model.y[color_id - 1]);
            model.model.elements_coefficients.push_back(-1.0);

            model.model.constraints_lower_bounds.push_back(-std::numeric_limits<double>::infinity());
            model.model.constraints_upper_bounds.push_back(0);
        }
    }

    return model;
}

Solution retrieve_solution(
        const Instance& instance,
        const ModelAssignment& model,
        const std::vector<double>& milp_solution)
{
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();
    ColorId upper_bound = graph.highest_degree() + 1;

    Solution solution(instance);
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
            double x = milp_solution[model.x[vertex_id][color_id]];
            if (x > 0.5)
                solution.set(vertex_id, color_id);
        }
    }
    return solution;
}

#ifdef CBC_FOUND

class EventHandlerAssignment: public CbcEventHandler
{

public:

    virtual CbcAction event(CbcEvent which_event);

    EventHandlerAssignment(
            const Instance& instance,
            const MilpAssignmentParameters& parameters,
            const ModelAssignment& milp_model,
            Output& output,
            AlgorithmFormatter& algorithm_formatter):
        CbcEventHandler(),
        instance_(instance),
        parameters_(parameters),
        milp_model_(milp_model),
        output_(output),
        algorithm_formatter_(algorithm_formatter) { }

    virtual ~EventHandlerAssignment() { }

    EventHandlerAssignment(const EventHandlerAssignment& rhs):
        CbcEventHandler(rhs),
        instance_(rhs.instance_),
        parameters_(rhs.parameters_),
        milp_model_(rhs.milp_model_),
        output_(rhs.output_),
        algorithm_formatter_(rhs.algorithm_formatter_) { }

    virtual CbcEventHandler* clone() const { return new EventHandlerAssignment(*this); }

private:

    const Instance& instance_;
    const MilpAssignmentParameters& parameters_;
    const ModelAssignment& milp_model_;
    Output& output_;
    AlgorithmFormatter& algorithm_formatter_;

};

CbcEventHandler::CbcAction EventHandlerAssignment::event(CbcEvent which_event)
{
    // Not in subtree.
    if ((model_->specialOptions() & 2048) != 0)
        return noAction;
    const CbcModel& cbc_model = *model_;

    int number_of_nodes = mathoptsolverscmake::get_number_of_nodes(cbc_model);

    // Retrieve solution.
    double milp_objective_value = mathoptsolverscmake::get_solution_value(cbc_model);
    if (!output_.solution.feasible()
            || output_.solution.number_of_colors() > milp_objective_value) {
        std::vector<double> milp_solution = mathoptsolverscmake::get_solution(cbc_model);
        Solution solution = retrieve_solution(instance_, milp_model_, milp_solution);
        algorithm_formatter_.update_solution(solution, "node " + std::to_string(number_of_nodes));
    }

    // Retrieve bound.
    ColorId bound = std::ceil(mathoptsolverscmake::get_bound(cbc_model) - 1e-5);
    algorithm_formatter_.update_bound(bound, "node " + std::to_string(number_of_nodes));

    // Check end.
    if (parameters_.timer.needs_to_end())
        return stop;

    return noAction;
}

#endif

#ifdef XPRESS_FOUND

struct XpressCallbackUser
{
    const Instance& instance;
    const MilpAssignmentParameters& parameters;
    Output& output;
    AlgorithmFormatter& algorithm_formatter;
};

void xpress_callback_assignment(
        XPRSprob xpress_model,
        void* user,
        int*)
{
    const XpressCallbackUser& d = *(const XpressCallbackUser*)(user);

    // Retrieve solution.
    double milp_objective_value = mathoptsolverscmake::get_solution_value(xpress_model);
    if (!d.output.solution.feasible()
            || d.output.solution.number_of_colors() > milp_objective_value) {
        std::vector<double> milp_solution = mathoptsolverscmake::get_solution(xpress_model);
        Solution solution = retrieve_solution(d.instance, milp_solution);
        d.algorithm_formatter.update_solution(solution, "");
    }

    // Retrieve bound.
    ColorId bound = std::ceil(mathoptsolverscmake::get_bound(xpress_model) - 1e-5);
    d.algorithm_formatter.update_bound(bound, "");

    // Check end.
    if (d.parameters.timer.needs_to_end())
        XPRSinterrupt(xpress_model, XPRS_STOP_USER);
};

#endif

}

Output coloringsolver::milp_assignment(
        const Instance& instance,
        const MilpAssignmentParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP - Assignment model");

    algorithm_formatter.print_header();

    ModelAssignment milp_model = create_milp_model_assignment(instance, parameters);

    std::vector<double> milp_solution;
    double milp_bound = 0;

    if (parameters.solver == mathoptsolverscmake::SolverName::Cbc) {
#ifdef CBC_FOUND
        OsiCbcSolverInterface osi_solver;
        CbcModel cbc_model(osi_solver);
        mathoptsolverscmake::reduce_printout(cbc_model);
        mathoptsolverscmake::set_time_limit(cbc_model, parameters.timer.remaining_time());
        mathoptsolverscmake::load(cbc_model, milp_model.model);
        EventHandlerAssignment cbc_event_handler(instance, parameters, milp_model, output, algorithm_formatter);
        cbc_model.passInEventHandler(&cbc_event_handler);
        mathoptsolverscmake::solve(cbc_model);
        milp_solution = mathoptsolverscmake::get_solution(cbc_model);
        milp_bound = mathoptsolverscmake::get_bound(cbc_model);
#else
        throw std::invalid_argument("");
#endif

    } else if (parameters.solver == mathoptsolverscmake::SolverName::Highs) {
#ifdef HIGHS_FOUND
        Highs highs;
        mathoptsolverscmake::reduce_printout(highs);
        mathoptsolverscmake::set_time_limit(highs, parameters.timer.remaining_time());
        mathoptsolverscmake::set_log_file(highs, "highs.log");
        mathoptsolverscmake::load(highs, milp_model.model);
        highs.setCallback([
                &instance,
                &parameters,
                &milp_model,
                &output,
                &algorithm_formatter](
                    const int,
                    const std::string& message,
                    const HighsCallbackOutput* highs_output,
                    HighsCallbackInput* highs_input,
                    void*)
                {
                    if (!highs_output->mip_solution.empty()) {
                        // Retrieve solution.
                        double milp_objective_value = highs_output->mip_primal_bound;
                        if (!output.solution.feasible()
                                || output.solution.number_of_colors() > milp_objective_value) {
                            Solution solution = retrieve_solution(instance, milp_model, highs_output->mip_solution);
                            algorithm_formatter.update_solution(solution, "node " + std::to_string(highs_output->mip_node_count));
                        }

                        // Retrieve bound.
                        ColorId bound = std::ceil(highs_output->mip_dual_bound - 1e-5);
                        if (bound != std::numeric_limits<double>::infinity())
                            algorithm_formatter.update_bound(bound, "node " + std::to_string(highs_output->mip_node_count));
                    }

                    // Check end.
                    if (parameters.timer.needs_to_end())
                        highs_input->user_interrupt = 1;
                },
                nullptr);
        HighsStatus highs_status;
        highs_status = highs.startCallback(HighsCallbackType::kCallbackMipImprovingSolution);
        highs_status = highs.startCallback(HighsCallbackType::kCallbackMipInterrupt);
        mathoptsolverscmake::solve(highs);
        milp_solution = mathoptsolverscmake::get_solution(highs);
        milp_bound = mathoptsolverscmake::get_bound(highs);
#else
        throw std::invalid_argument("");
#endif

    } else if (parameters.solver == mathoptsolverscmake::SolverName::Xpress) {
#ifdef XPRESS_FOUND
        XPRSprob xpress_model;
        XPRScreateprob(&xpress_model);
        mathoptsolverscmake::set_time_limit(xpress_model, parameters.timer.remaining_time());
        mathoptsolverscmake::set_log_file(xpress_model, "xpress.log");
        mathoptsolverscmake::load(xpress_model, milp_model);
        //mathoptsolverscmake::write_mps(xpress_model, "kpc.mps");
        XpressCallbackUser xpress_callback_user{instance, parameters, output, algorithm_formatter};
        XPRSaddcbprenode(xpress_model, xpress_callback_assignment, (void*)&xpress_callback_user, 0);
        mathoptsolverscmake::solve(xpress_model);
        milp_solution = mathoptsolverscmake::get_solution(xpress_model);
        milp_bound = mathoptsolverscmake::get_bound(xpress_model);
        XPRSdestroyprob(xpress_model);
#else
        throw std::invalid_argument("");
#endif

    } else {
        throw std::invalid_argument("");
    }

    // Retrieve solution.
    Solution solution = retrieve_solution(instance, milp_model, milp_solution);
    algorithm_formatter.update_solution(solution, "");

    // Retrieve bound.
    ColorId bound = std::ceil(milp_bound - 1e-5);
    algorithm_formatter.update_bound(bound, "");

    algorithm_formatter.end();
    return output;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Representatives ILP model ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace
{

struct ModelRepresentatives
{
    /** Model. */
    mathoptsolverscmake::MilpModel model;

    /** x[u][v] = 1 iff vertex 'u' is represented by vertex 'v'. */
    std::vector<std::vector<int>> x;
};

ModelRepresentatives create_milp_model_representatives(
        const Instance& instance)
{
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();
    optimizationtools::AdjacencyListGraph complementary_graph = graph.complementary();

    ModelRepresentatives model;

    /////////////////////////////
    // Variables and objective //
    /////////////////////////////

    model.model.objective_direction = mathoptsolverscmake::ObjectiveDirection::Minimize;

    // Variables x.
    model.x = std::vector<std::vector<int>>(graph.number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        model.x[vertex_id] = std::vector<int>(graph.number_of_vertices());
        for (VertexId vertex_2_id = 0;
                vertex_2_id < graph.number_of_vertices();
                ++vertex_2_id) {
            model.x[vertex_id][vertex_2_id] = model.model.variables_lower_bounds.size();
            model.model.variables_lower_bounds.push_back(0);
            model.model.variables_upper_bounds.push_back(1);
            model.model.variables_types.push_back(mathoptsolverscmake::VariableType::Binary);
            if (vertex_id == vertex_2_id) {
                model.model.objective_coefficients.push_back(1);
            } else {
                model.model.objective_coefficients.push_back(0);
            }
        }
    }

    /////////////////
    // Constraints //
    /////////////////

    // Constraints: every vertex must have a representative (possibly itself).
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        model.model.constraints_starts.push_back(model.model.elements_variables.size());

        model.model.elements_variables.push_back(model.x[vertex_id][vertex_id]);
        model.model.elements_coefficients.push_back(1.0);
        for (auto it = complementary_graph.neighbors_begin(vertex_id);
                it != complementary_graph.neighbors_end(vertex_id);
                ++it) {
            model.model.elements_variables.push_back(model.x[*it][vertex_id]);
            model.model.elements_coefficients.push_back(1.0);
        }

        model.model.constraints_lower_bounds.push_back(1);
        model.model.constraints_upper_bounds.push_back(std::numeric_limits<double>::infinity());
    }

    // Constraint: a representative cannot represent both endpoints of an edge.
    for (EdgeId edge_id = 0; edge_id < graph.number_of_edges(); ++edge_id) {
        for (VertexId vertex_id = 0;
                vertex_id < graph.number_of_vertices();
                ++vertex_id) {
            if (vertex_id == graph.first_end(edge_id)
                    || vertex_id == graph.second_end(edge_id)) {
                continue;
            }

            model.model.constraints_starts.push_back(model.model.elements_variables.size());

            model.model.elements_variables.push_back(model.x[vertex_id][graph.first_end(edge_id)]);
            model.model.elements_coefficients.push_back(1.0);
            model.model.elements_variables.push_back(model.x[vertex_id][graph.second_end(edge_id)]);
            model.model.elements_coefficients.push_back(1.0);
            model.model.elements_variables.push_back(model.x[vertex_id][vertex_id]);
            model.model.elements_coefficients.push_back(-1.0);

            model.model.constraints_lower_bounds.push_back(-std::numeric_limits<double>::infinity());
            model.model.constraints_upper_bounds.push_back(0);
        }
    }

    return model;
}

Solution retrieve_solution(
        const Instance& instance,
        const ModelRepresentatives& model,
        const std::vector<double>& milp_solution)
{
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    Solution solution(instance);
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (VertexId vertex_2_id = 0;
                vertex_2_id < graph.number_of_vertices();
                ++vertex_2_id) {
            //std::cout << "vertex_id " << vertex_id
            //    << " vertex_2_id " << vertex_2_id
            //    << " x " << milp_solution[model.x[vertex_2_id][vertex_id]]
            //    << std::endl;
            if (milp_solution[model.x[vertex_2_id][vertex_id]] > 0.5) {
                solution.set(vertex_id, vertex_2_id);
                break;
            }
        }
    }
    return solution;
}

#ifdef CBC_FOUND

class EventHandlerRepresentatives: public CbcEventHandler
{

public:

    virtual CbcAction event(CbcEvent which_event);

    EventHandlerRepresentatives(
            const Instance& instance,
            const MilpParameters& parameters,
            const ModelRepresentatives& milp_model,
            Output& output,
            AlgorithmFormatter& algorithm_formatter):
        CbcEventHandler(),
        instance_(instance),
        parameters_(parameters),
        milp_model_(milp_model),
        output_(output),
        algorithm_formatter_(algorithm_formatter) { }

    virtual ~EventHandlerRepresentatives() { }

    EventHandlerRepresentatives(const EventHandlerRepresentatives& rhs):
        CbcEventHandler(rhs),
        instance_(rhs.instance_),
        parameters_(rhs.parameters_),
        milp_model_(rhs.milp_model_),
        output_(rhs.output_),
        algorithm_formatter_(rhs.algorithm_formatter_) { }

    virtual CbcEventHandler* clone() const { return new EventHandlerRepresentatives(*this); }

private:

    const Instance& instance_;
    const MilpParameters& parameters_;
    const ModelRepresentatives& milp_model_;
    Output& output_;
    AlgorithmFormatter& algorithm_formatter_;

};

CbcEventHandler::CbcAction EventHandlerRepresentatives::event(CbcEvent which_event)
{
    // Not in subtree.
    if ((model_->specialOptions() & 2048) != 0)
        return noAction;
    const CbcModel& cbc_model = *model_;

    int number_of_nodes = mathoptsolverscmake::get_number_of_nodes(cbc_model);

    // Retrieve solution.
    double milp_objective_value = mathoptsolverscmake::get_solution_value(cbc_model);
    if (!output_.solution.feasible()
            || output_.solution.number_of_colors() > milp_objective_value) {
        std::vector<double> milp_solution = mathoptsolverscmake::get_solution(cbc_model);
        Solution solution = retrieve_solution(instance_, milp_model_, milp_solution);
        algorithm_formatter_.update_solution(solution, "node " + std::to_string(number_of_nodes));
    }

    // Retrieve bound.
    ColorId bound = std::ceil(mathoptsolverscmake::get_bound(cbc_model) - 1e-5);
    algorithm_formatter_.update_bound(bound, "node " + std::to_string(number_of_nodes));

    // Check end.
    if (parameters_.timer.needs_to_end())
        return stop;

    return noAction;
}

#endif

#ifdef XPRESS_FOUND

struct XpressCallbackUser
{
    const Instance& instance;
    const MilpAssignmentParameters& parameters;
    Output& output;
    AlgorithmFormatter& algorithm_formatter;
};

void xpress_callback_representatives(
        XPRSprob xpress_model,
        void* user,
        int*)
{
    const XpressCallbackUser& d = *(const XpressCallbackUser*)(user);

    // Retrieve solution.
    double milp_objective_value = mathoptsolverscmake::get_solution_value(xpress_model);
    if (!d.output.solution.feasible()
            || d.output.solution.number_of_colors() > milp_objective_value) {
        std::vector<double> milp_solution = mathoptsolverscmake::get_solution(xpress_model);
        Solution solution = retrieve_solution(d.instance, milp_solution);
        d.algorithm_formatter.update_solution(solution, "");
    }

    // Retrieve bound.
    ColorId bound = std::ceil(mathoptsolverscmake::get_bound(xpress_model) - 1e-5);
    d.algorithm_formatter.update_bound(bound, "");

    // Check end.
    if (d.parameters.timer.needs_to_end())
        XPRSinterrupt(xpress_model, XPRS_STOP_USER);
};

#endif

}

Output coloringsolver::milp_representatives(
        const Instance& instance,
        const MilpParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP - Representative model");
    algorithm_formatter.print_header();

    ModelRepresentatives milp_model = create_milp_model_representatives(instance);

    std::vector<double> milp_solution;
    double milp_bound = 0;

    if (parameters.solver == mathoptsolverscmake::SolverName::Cbc) {
#ifdef CBC_FOUND
        OsiCbcSolverInterface osi_solver;
        CbcModel cbc_model(osi_solver);
        mathoptsolverscmake::reduce_printout(cbc_model);
        mathoptsolverscmake::set_time_limit(cbc_model, parameters.timer.remaining_time());
        mathoptsolverscmake::load(cbc_model, milp_model.model);
        EventHandlerRepresentatives cbc_event_handler(instance, parameters, milp_model, output, algorithm_formatter);
        cbc_model.passInEventHandler(&cbc_event_handler);
        mathoptsolverscmake::solve(cbc_model);
        milp_solution = mathoptsolverscmake::get_solution(cbc_model);
        milp_bound = mathoptsolverscmake::get_bound(cbc_model);
#else
        throw std::invalid_argument("");
#endif

    } else if (parameters.solver == mathoptsolverscmake::SolverName::Highs) {
#ifdef HIGHS_FOUND
        Highs highs;
        mathoptsolverscmake::reduce_printout(highs);
        mathoptsolverscmake::set_time_limit(highs, parameters.timer.remaining_time());
        mathoptsolverscmake::set_log_file(highs, "highs.log");
        mathoptsolverscmake::load(highs, milp_model.model);
        highs.setCallback([
                &instance,
                &parameters,
                &milp_model,
                &output,
                &algorithm_formatter](
                    const int,
                    const std::string& message,
                    const HighsCallbackOutput* highs_output,
                    HighsCallbackInput* highs_input,
                    void*)
                {
                    if (!highs_output->mip_solution.empty()) {
                        // Retrieve solution.
                        double milp_objective_value = highs_output->mip_primal_bound;
                        if (!output.solution.feasible()
                                || output.solution.number_of_colors() > milp_objective_value) {
                            Solution solution = retrieve_solution(instance, milp_model, highs_output->mip_solution);
                            algorithm_formatter.update_solution(solution, "node " + std::to_string(highs_output->mip_node_count));
                        }

                        // Retrieve bound.
                        ColorId bound = std::ceil(highs_output->mip_dual_bound - 1e-5);
                        if (bound != std::numeric_limits<double>::infinity())
                            algorithm_formatter.update_bound(bound, "node " + std::to_string(highs_output->mip_node_count));
                    }

                    // Check end.
                    if (parameters.timer.needs_to_end())
                        highs_input->user_interrupt = 1;
                },
                nullptr);
        HighsStatus highs_status;
        highs_status = highs.startCallback(HighsCallbackType::kCallbackMipImprovingSolution);
        highs_status = highs.startCallback(HighsCallbackType::kCallbackMipInterrupt);
        mathoptsolverscmake::solve(highs);
        milp_solution = mathoptsolverscmake::get_solution(highs);
        milp_bound = mathoptsolverscmake::get_bound(highs);
#else
        throw std::invalid_argument("");
#endif

    } else if (parameters.solver == mathoptsolverscmake::SolverName::Xpress) {
#ifdef XPRESS_FOUND
        XPRSprob xpress_model;
        XPRScreateprob(&xpress_model);
        mathoptsolverscmake::set_time_limit(xpress_model, parameters.timer.remaining_time());
        mathoptsolverscmake::set_log_file(xpress_model, "xpress.log");
        mathoptsolverscmake::load(xpress_model, milp_model);
        //mathoptsolverscmake::write_mps(xpress_model, "kpc.mps");
        XpressCallbackUser xpress_callback_user{instance, parameters, output, algorithm_formatter};
        XPRSaddcbprenode(xpress_model, xpress_callback_representatives, (void*)&xpress_callback_user, 0);
        mathoptsolverscmake::solve(xpress_model);
        milp_solution = mathoptsolverscmake::get_solution(xpress_model);
        milp_bound = mathoptsolverscmake::get_bound(xpress_model);
        XPRSdestroyprob(xpress_model);
#else
        throw std::invalid_argument("");
#endif

    } else {
        throw std::invalid_argument("");
    }

    // Retrieve solution.
    Solution solution = retrieve_solution(instance, milp_model, milp_solution);
    algorithm_formatter.update_solution(solution, "");

    // Retrieve bound.
    ColorId bound = std::ceil(milp_bound - 1e-5);
    algorithm_formatter.update_bound(bound, "");

    algorithm_formatter.end();
    return output;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Partial-ordering based ILP model ///////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace
{

struct ModelPartialOrdering
{
    /** Model. */
    mathoptsolverscmake::MilpModel model;

    /**
     * x[v][c] == 1 iff vertex 'v' has color 'c'.
     *
     * These variables are only used in the hybrid model.
     */
    std::vector<std::vector<int>> x;

    /** y[v][c] == 1 iff vertex 'v' has color > 'c'. */
    std::vector<std::vector<int>> y;

    /** y[v][c] == 1 iff vertex 'v' has color < 'c'. */
    std::vector<std::vector<int>> z;
};

ModelPartialOrdering create_milp_model_partial_ordering(
        const Instance& instance,
        const MilpPartialOrderingParameters& parameters)
{
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    ModelPartialOrdering model;

    /////////////////////////////
    // Variables and objective //
    /////////////////////////////

    model.model.objective_direction = mathoptsolverscmake::ObjectiveDirection::Minimize;

    ColorId upper_bound = graph.highest_degree() + 1;

    // Variables y and z.
    model.y = std::vector<std::vector<int>>(graph.number_of_vertices());
    model.z = std::vector<std::vector<int>>(graph.number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        model.y[vertex_id] = std::vector<int>(upper_bound);
        model.z[vertex_id] = std::vector<int>(upper_bound);
        for (ColorId color_id = 0;
                color_id < upper_bound;
                ++color_id) {
            model.y[vertex_id][color_id] = model.model.variables_lower_bounds.size();
            model.model.variables_lower_bounds.push_back(0);
            if (color_id == upper_bound - 1) {
                model.model.variables_upper_bounds.push_back(0);
            } else {
                model.model.variables_upper_bounds.push_back(1);
            }
            model.model.variables_types.push_back(mathoptsolverscmake::VariableType::Binary);
            if (vertex_id == 0) {
                model.model.objective_coefficients.push_back(1);
            } else {
                model.model.objective_coefficients.push_back(0);
            }

            model.z[vertex_id][color_id] = model.model.variables_lower_bounds.size();
            model.model.variables_lower_bounds.push_back(0);
            if (color_id == 0) {
                model.model.variables_upper_bounds.push_back(0);
            } else {
                model.model.variables_upper_bounds.push_back(1);
            }
            model.model.variables_types.push_back(mathoptsolverscmake::VariableType::Binary);
            model.model.objective_coefficients.push_back(0);
        }
    }

    if (parameters.hybrid) {
        model.x = std::vector<std::vector<int>>(graph.number_of_vertices());
        for (VertexId vertex_id = 0;
                vertex_id < graph.number_of_vertices();
                ++vertex_id) {
            model.x[vertex_id] = std::vector<int>(upper_bound);
            for (ColorId color_id = 0;
                    color_id < upper_bound;
                    ++color_id) {
                model.x[vertex_id][color_id] = model.model.variables_lower_bounds.size();
                model.model.variables_lower_bounds.push_back(0);
                model.model.variables_upper_bounds.push_back(1);
                model.model.variables_types.push_back(mathoptsolverscmake::VariableType::Binary);
                model.model.objective_coefficients.push_back(0);
            }
        }
    }

    /////////////////
    // Constraints //
    /////////////////

    // If the color of vertex 'v' is '> c + 1', then it is '> c'.
    // y[v][c + 1] <= y[v][c]
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound - 1; ++color_id) {
            model.model.constraints_starts.push_back(model.model.elements_variables.size());

            model.model.elements_variables.push_back(model.y[vertex_id][color_id + 1]);
            model.model.elements_coefficients.push_back(1.0);
            model.model.elements_variables.push_back(model.y[vertex_id][color_id]);
            model.model.elements_coefficients.push_back(-1.0);

            model.model.constraints_lower_bounds.push_back(-std::numeric_limits<double>::infinity());
            model.model.constraints_upper_bounds.push_back(0);
        }
    }

    // A vertex has a color either '> c' or '< c + 1'.
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound - 1; ++color_id) {
            model.model.constraints_starts.push_back(model.model.elements_variables.size());

            model.model.elements_variables.push_back(model.y[vertex_id][color_id]);
            model.model.elements_coefficients.push_back(1.0);
            model.model.elements_variables.push_back(model.z[vertex_id][color_id + 1]);
            model.model.elements_coefficients.push_back(1.0);

            model.model.constraints_lower_bounds.push_back(1);
            model.model.constraints_upper_bounds.push_back(1);
        }
    }

    // Prevent assigning the same color to two adjacent vertices.
    if (!parameters.hybrid) {
        // y[v1][c] + z[v1][c] + y[v2][c] + z[v2][c] >= 1
        for (EdgeId edge_id = 0; edge_id < graph.number_of_edges(); ++edge_id) {
            VertexId vertex_1_id = graph.first_end(edge_id);
            VertexId vertex_2_id = graph.second_end(edge_id);
            for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
                model.model.constraints_starts.push_back(model.model.elements_variables.size());

                model.model.elements_variables.push_back(model.y[vertex_1_id][color_id]);
                model.model.elements_coefficients.push_back(1.0);
                model.model.elements_variables.push_back(model.z[vertex_1_id][color_id]);
                model.model.elements_coefficients.push_back(1.0);
                model.model.elements_variables.push_back(model.y[vertex_2_id][color_id]);
                model.model.elements_coefficients.push_back(1.0);
                model.model.elements_variables.push_back(model.z[vertex_2_id][color_id]);
                model.model.elements_coefficients.push_back(1.0);

                model.model.constraints_lower_bounds.push_back(1);
                model.model.constraints_upper_bounds.push_back(std::numeric_limits<double>::infinity());
            }
        }

    } else {
        // Link x with y and z.
        for (VertexId vertex_id = 0;
                vertex_id < graph.number_of_vertices();
                ++vertex_id) {
            for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
                model.model.constraints_starts.push_back(model.model.elements_variables.size());

                model.model.elements_variables.push_back(model.x[vertex_id][color_id]);
                model.model.elements_coefficients.push_back(1.0);
                model.model.elements_variables.push_back(model.y[vertex_id][color_id]);
                model.model.elements_coefficients.push_back(1.0);
                model.model.elements_variables.push_back(model.z[vertex_id][color_id]);
                model.model.elements_coefficients.push_back(1.0);

                model.model.constraints_lower_bounds.push_back(1);
                model.model.constraints_upper_bounds.push_back(1);
            }
        }

        // Prevent assigning the same color to two adjacent vertices.
        for (EdgeId edge_id = 0; edge_id < graph.number_of_edges(); ++edge_id) {
            VertexId vertex_1_id = graph.first_end(edge_id);
            VertexId vertex_2_id = graph.second_end(edge_id);
            for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
                model.model.constraints_starts.push_back(model.model.elements_variables.size());

                model.model.elements_variables.push_back(model.x[vertex_1_id][color_id]);
                model.model.elements_coefficients.push_back(1.0);
                model.model.elements_variables.push_back(model.x[vertex_2_id][color_id]);
                model.model.elements_coefficients.push_back(1.0);

                model.model.constraints_lower_bounds.push_back(0);
                model.model.constraints_upper_bounds.push_back(1);
            }
        }
    }

    // y[0][c] - y[v][c] >= 0
    for (VertexId vertex_id = 1;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound - 1; ++color_id) {
            model.model.constraints_starts.push_back(model.model.elements_variables.size());

            model.model.elements_variables.push_back(model.y[0][color_id]);
            model.model.elements_coefficients.push_back(1.0);
            model.model.elements_variables.push_back(model.y[vertex_id][color_id]);
            model.model.elements_coefficients.push_back(-1.0);

            model.model.constraints_lower_bounds.push_back(0);
            model.model.constraints_upper_bounds.push_back(std::numeric_limits<double>::infinity());
        }
    }

    return model;
}

Solution retrieve_solution(
        const Instance& instance,
        const ModelPartialOrdering& model,
        const std::vector<double>& milp_solution)
{
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();
    ColorId upper_bound = graph.highest_degree() + 1;

    Solution solution(instance);
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
            if (milp_solution[model.y[vertex_id][color_id]]
                    + milp_solution[model.z[vertex_id][color_id]] < 0.5)
                solution.set(vertex_id, color_id);
        }
    }
    return solution;
}

#ifdef CBC_FOUND

class EventHandlerPartialOrdering: public CbcEventHandler
{

public:

    virtual CbcAction event(CbcEvent which_event);

    EventHandlerPartialOrdering(
            const Instance& instance,
            const MilpPartialOrderingParameters& parameters,
            const ModelPartialOrdering& milp_model,
            Output& output,
            AlgorithmFormatter& algorithm_formatter):
        CbcEventHandler(),
        instance_(instance),
        parameters_(parameters),
        milp_model_(milp_model),
        output_(output),
        algorithm_formatter_(algorithm_formatter) { }

    virtual ~EventHandlerPartialOrdering() { }

    EventHandlerPartialOrdering(const EventHandlerPartialOrdering& rhs):
        CbcEventHandler(rhs),
        instance_(rhs.instance_),
        parameters_(rhs.parameters_),
        milp_model_(rhs.milp_model_),
        output_(rhs.output_),
        algorithm_formatter_(rhs.algorithm_formatter_) { }

    virtual CbcEventHandler* clone() const { return new EventHandlerPartialOrdering(*this); }

private:

    const Instance& instance_;
    const MilpPartialOrderingParameters& parameters_;
    const ModelPartialOrdering& milp_model_;
    Output& output_;
    AlgorithmFormatter& algorithm_formatter_;

};

CbcEventHandler::CbcAction EventHandlerPartialOrdering::event(CbcEvent which_event)
{
    // Not in subtree.
    if ((model_->specialOptions() & 2048) != 0)
        return noAction;
    const CbcModel& cbc_model = *model_;

    int number_of_nodes = mathoptsolverscmake::get_number_of_nodes(cbc_model);

    // Retrieve solution.
    double milp_objective_value = mathoptsolverscmake::get_solution_value(cbc_model);
    if (!output_.solution.feasible()
            || output_.solution.number_of_colors() > milp_objective_value) {
        std::vector<double> milp_solution = mathoptsolverscmake::get_solution(cbc_model);
        Solution solution = retrieve_solution(instance_, milp_model_, milp_solution);
        algorithm_formatter_.update_solution(solution, "node " + std::to_string(number_of_nodes));
    }

    // Retrieve bound.
    ColorId bound = std::ceil(mathoptsolverscmake::get_bound(cbc_model) - 1e-5) + 1;
    algorithm_formatter_.update_bound(bound, "node " + std::to_string(number_of_nodes));

    // Check end.
    if (parameters_.timer.needs_to_end())
        return stop;

    return noAction;
}

#endif

#ifdef XPRESS_FOUND

struct XpressCallbackUser
{
    const Instance& instance;
    const MilpPartialOrderingParameters& parameters;
    Output& output;
    AlgorithmFormatter& algorithm_formatter;
};

void xpress_callback_partial_ordering(
        XPRSprob xpress_model,
        void* user,
        int*)
{
    const XpressCallbackUser& d = *(const XpressCallbackUser*)(user);

    // Retrieve solution.
    double milp_objective_value = mathoptsolverscmake::get_solution_value(xpress_model);
    if (!d.output.solution.feasible()
            || d.output.solution.number_of_colors() > milp_objective_value) {
        std::vector<double> milp_solution = mathoptsolverscmake::get_solution(xpress_model);
        Solution solution = retrieve_solution(d.instance, milp_solution);
        d.algorithm_formatter.update_solution(solution, "");
    }

    // Retrieve bound.
    ColorId bound = std::ceil(mathoptsolverscmake::get_bound(xpress_model) - 1e-5) + 1;
    d.algorithm_formatter.update_bound(bound, "");

    // Check end.
    if (d.parameters.timer.needs_to_end())
        XPRSinterrupt(xpress_model, XPRS_STOP_USER);
};

#endif

}

Output coloringsolver::milp_partial_ordering(
        const Instance& instance,
        const MilpPartialOrderingParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP - Partial ordering model");

    algorithm_formatter.print_header();

    ModelPartialOrdering milp_model = create_milp_model_partial_ordering(instance, parameters);

    std::vector<double> milp_solution;
    double milp_bound = 0;

    if (parameters.solver == mathoptsolverscmake::SolverName::Cbc) {
#ifdef CBC_FOUND
        OsiCbcSolverInterface osi_solver;
        CbcModel cbc_model(osi_solver);
        mathoptsolverscmake::reduce_printout(cbc_model);
        mathoptsolverscmake::set_time_limit(cbc_model, parameters.timer.remaining_time());
        mathoptsolverscmake::load(cbc_model, milp_model.model);
        EventHandlerPartialOrdering cbc_event_handler(instance, parameters, milp_model, output, algorithm_formatter);
        cbc_model.passInEventHandler(&cbc_event_handler);
        mathoptsolverscmake::solve(cbc_model);
        milp_solution = mathoptsolverscmake::get_solution(cbc_model);
        milp_bound = mathoptsolverscmake::get_bound(cbc_model);
#else
        throw std::invalid_argument("");
#endif

    } else if (parameters.solver == mathoptsolverscmake::SolverName::Highs) {
#ifdef HIGHS_FOUND
        Highs highs;
        mathoptsolverscmake::reduce_printout(highs);
        mathoptsolverscmake::set_time_limit(highs, parameters.timer.remaining_time());
        mathoptsolverscmake::set_log_file(highs, "highs.log");
        mathoptsolverscmake::load(highs, milp_model.model);
        highs.setCallback([
                &instance,
                &parameters,
                &milp_model,
                &output,
                &algorithm_formatter](
                    const int,
                    const std::string& message,
                    const HighsCallbackOutput* highs_output,
                    HighsCallbackInput* highs_input,
                    void*)
                {
                    if (!highs_output->mip_solution.empty()) {
                        // Retrieve solution.
                        double milp_objective_value = highs_output->mip_primal_bound;
                        if (!output.solution.feasible()
                                || output.solution.number_of_colors() > milp_objective_value) {
                            Solution solution = retrieve_solution(instance, milp_model, highs_output->mip_solution);
                            algorithm_formatter.update_solution(solution, "node " + std::to_string(highs_output->mip_node_count));
                        }

                        // Retrieve bound.
                        ColorId bound = std::ceil(highs_output->mip_dual_bound - 1e-5) + 1;
                        if (bound != std::numeric_limits<double>::infinity())
                            algorithm_formatter.update_bound(bound, "node " + std::to_string(highs_output->mip_node_count));
                    } else {
                        // Check end.
                        if (parameters.timer.needs_to_end())
                            highs_input->user_interrupt = 1;
                    }
                },
                nullptr);
        HighsStatus highs_status;
        highs_status = highs.startCallback(HighsCallbackType::kCallbackMipImprovingSolution);
        highs_status = highs.startCallback(HighsCallbackType::kCallbackMipInterrupt);
        mathoptsolverscmake::solve(highs);
        milp_solution = mathoptsolverscmake::get_solution(highs);
        milp_bound = mathoptsolverscmake::get_bound(highs);
#else
        throw std::invalid_argument("");
#endif

    } else if (parameters.solver == mathoptsolverscmake::SolverName::Xpress) {
#ifdef XPRESS_FOUND
        XPRSprob xpress_model;
        XPRScreateprob(&xpress_model);
        mathoptsolverscmake::set_time_limit(xpress_model, parameters.timer.remaining_time());
        mathoptsolverscmake::set_log_file(xpress_model, "xpress.log");
        mathoptsolverscmake::load(xpress_model, milp_model);
        //mathoptsolverscmake::write_mps(xpress_model, "kpc.mps");
        XpressCallbackUser xpress_callback_user{instance, parameters, output, algorithm_formatter};
        XPRSaddcbprenode(xpress_model, xpress_callback_partial_ordering, (void*)&xpress_callback_user, 0);
        mathoptsolverscmake::solve(xpress_model);
        milp_solution = mathoptsolverscmake::get_solution(xpress_model);
        milp_bound = mathoptsolverscmake::get_bound(xpress_model);
        XPRSdestroyprob(xpress_model);
#else
        throw std::invalid_argument("");
#endif

    } else {
        throw std::invalid_argument("");
    }

    // Retrieve solution.
    Solution solution = retrieve_solution(instance, milp_model, milp_solution);
    algorithm_formatter.update_solution(solution, "");

    // Retrieve bound.
    ColorId bound = std::ceil(milp_bound - 1e-5) + 1;
    algorithm_formatter.update_bound(bound, "");

    algorithm_formatter.end();
    return output;
}
