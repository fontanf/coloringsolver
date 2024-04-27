#if CPLEX_FOUND

#include "coloringsolver/algorithms/milp_cplex.hpp"

#include "coloringsolver/algorithm_formatter.hpp"

#include <ilcplex/ilocplex.h>

using namespace coloringsolver;

ILOSTLBEGIN

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Assignment-based ILP model //////////////////////////
////////////////////////////////////////////////////////////////////////////////

ILOMIPINFOCALLBACK5(loggingCallbackAssignment,
                    const Instance&, instance,
                    AlgorithmFormatter&, algorithm_formatter,
                    Output&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, x)
{
    ColorId lb = std::ceil(getBestObjValue() - FFOT_TOL);
    algorithm_formatter.update_bound(lb, "");

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible()
            || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId vertex_id = 0;
                vertex_id < instance.graph().number_of_vertices();
                ++vertex_id) {
            IloNumArray val(x[vertex_id].getEnv());
            getIncumbentValues(val, x[vertex_id]);
            for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
                if (val[color_id] > 0.5)
                    solution.set(vertex_id, color_id);
        }
        algorithm_formatter.update_solution(solution, "");
    }
}

const Output coloringsolver::milp_assignment_cplex(
        const Instance& instance,
        const MilpAssignmentCplexParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP - Assignment model (CPLEX)");
    algorithm_formatter.print_header();

    if (instance.adjacency_list_graph() == nullptr) {
        throw std::runtime_error(
                "The 'milp_assignment_cplex' algorithm requires an AdjacencyListGraph.");
    }
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = graph.highest_degree() + 1;

    // Variables
    // x[v][c] == 1 iff vertex v has color c.
    std::vector<IloNumVarArray> x;
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id)
        x.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    // y[c] == 1 iff color c is used.
    IloNumVarArray y(env, upper_bound, 0, 1, ILOBOOL);

    // Objective
    IloExpr expr(env);
    for (ColorId color_id = 0; color_id < upper_bound; ++ color_id)
        expr += y[color_id];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // Edge constraints
    for (EdgeId edge_id = 0; edge_id < graph.number_of_edges(); ++edge_id) {
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
            IloExpr expr(env);
            expr += x[graph.first_end(edge_id)][color_id];
            expr += x[graph.second_end(edge_id)][color_id];
            expr -= y[color_id];
            model.add(expr <= 0);
        }
    }

    // Each vertex must have a color.
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        IloExpr expr(env);
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
            expr += x[vertex_id][color_id];
        model.add(expr == 1);
    }

    if (parameters.break_symmetries) {
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
            IloExpr expr(env);
            expr += y[color_id];
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id) {
                expr -= x[vertex_id][color_id];
            }
            model.add(expr <= 0);
        }
        for (ColorId color_id = 1; color_id < upper_bound; ++color_id)
            model.add(y[color_id] <= y[color_id - 1]);
    }

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.timer.remaining_time() != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.timer.remaining_time());

    // Callback
    cplex.use(loggingCallbackAssignment(
                env,
                instance,
                algorithm_formatter,
                output,
                upper_bound,
                x));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id)
                for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
                    if (cplex.getValue(x[vertex_id][color_id]) > 0.5)
                        solution.set(vertex_id, color_id);
            algorithm_formatter.update_solution(solution, "");
        }
        algorithm_formatter.update_bound(
                output.solution.number_of_colors(),
                "");
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id)
                for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
                    if (cplex.getValue(x[vertex_id][color_id]) > 0.5)
                        solution.set(vertex_id, color_id);
            algorithm_formatter.update_solution(solution, "");
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(lb, "");
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(lb, "");
    }

    env.end();

    algorithm_formatter.end();
    return output;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Representatives ILP model ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

ILOMIPINFOCALLBACK5(loggingCallbackRepresentatives,
                    const Instance&, instance,
                    AlgorithmFormatter&, algorithm_formatter,
                    Output&, output,
                    const optimizationtools::AdjacencyListGraph&, complementary_graph,
                    std::vector<IloNumVarArray>&, x)
{
    ColorId lb = std::ceil(getBestObjValue() - FFOT_TOL);
    algorithm_formatter.update_bound(lb, "");

    if (!hasIncumbent())
        return;

    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();
    if (!output.solution.feasible()
            || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId vertex_id_1 = 0;
                vertex_id_1 < graph.number_of_vertices();
                ++vertex_id_1) {
            for (VertexId vertex_id_2 = 0;
                    vertex_id_2 < graph.number_of_vertices();
                    ++vertex_id_2) {
                if (getIncumbentValue(x[vertex_id_2][vertex_id_1]) > 0.5) {
                    solution.set(vertex_id_1, vertex_id_2);
                    break;
                }
            }
        }
        algorithm_formatter.update_solution(solution, "");
    }
}

const Output coloringsolver::milp_representatives_cplex(
        const Instance& instance,
        const MilpRepresentativesCplexParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP - Representative model (CPLEX)");
    algorithm_formatter.print_header();

    if (instance.adjacency_list_graph() == nullptr) {
        throw std::runtime_error(
                "The 'milp_representatives_cplex' algorithm requires an AdjacencyListGraph.");
    }
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    IloEnv env;
    IloModel model(env);

    // Build complementary graph.
    optimizationtools::AdjacencyListGraph complementary_graph = graph.complementary();

    // Variables
    // x[u][v] == 1 iff vertex v is represented by u.
    std::vector<IloNumVarArray> x(graph.number_of_vertices());
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id)
        x[vertex_id] = IloNumVarArray(env, graph.number_of_vertices(), 0, 1, ILOBOOL);

    // Objective
    IloExpr expr(env);
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id)
        expr += x[vertex_id][vertex_id];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // Every vertex v must have a representative (possibly v itself).
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        IloExpr expr(env);
        expr += x[vertex_id][vertex_id];
        for (auto it = complementary_graph.neighbors_begin(vertex_id);
                it != complementary_graph.neighbors_end(vertex_id);
                ++it) {
            expr += x[*it][vertex_id];
        }
        model.add(expr >= 1);
    }

    // A representative can not represent both endpoints of an edge.
    for (EdgeId edge_id = 0; edge_id < graph.number_of_edges(); ++edge_id) {
        for (VertexId vertex_id = 0;
                vertex_id < graph.number_of_vertices();
                ++vertex_id) {
            model.add(
                    x[vertex_id][graph.first_end(edge_id)]
                    + x[vertex_id][graph.second_end(edge_id)]
                    <= x[vertex_id][vertex_id]);
        }
    }

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.timer.remaining_time() != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.timer.remaining_time());

    // Callback
    cplex.use(loggingCallbackRepresentatives(
                env,
                instance,
                algorithm_formatter,
                output,
                complementary_graph,
                x));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id_1 = 0;
                    vertex_id_1 < graph.number_of_vertices();
                    ++vertex_id_1) {
                for (VertexId vertex_id_2 = 0;
                        vertex_id_2 < graph.number_of_vertices();
                        ++vertex_id_2) {
                    if (cplex.getValue(x[vertex_id_2][vertex_id_1]) > 0.5) {
                        solution.set(vertex_id_1, vertex_id_2);
                        break;
                    }
                }
            }
            algorithm_formatter.update_solution(solution, "");
        }
        algorithm_formatter.update_bound(
                output.solution.number_of_colors(),
                "");
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id_1 = 0;
                    vertex_id_1 < graph.number_of_vertices();
                    ++vertex_id_1) {
                for (VertexId vertex_id_2 = 0;
                        vertex_id_2 < graph.number_of_vertices();
                        ++vertex_id_2) {
                    if (cplex.getValue(x[vertex_id_2][vertex_id_1]) > 0.5) {
                        solution.set(vertex_id_1, vertex_id_2);
                        break;
                    }
                }
            }
            algorithm_formatter.update_solution(solution, "");
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(lb, "");
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(lb, "");
    }

    env.end();

    algorithm_formatter.end();
    return output;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Partial-ordering based ILP model ///////////////////////
////////////////////////////////////////////////////////////////////////////////

ILOMIPINFOCALLBACK6(loggingCallbackPartialOrdering,
                    const Instance&, instance,
                    AlgorithmFormatter&, algorithm_formatter,
                    Output&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, y,
                    std::vector<IloNumVarArray>&, z)
{
    ColorId lb = std::ceil(getBestObjValue() - FFOT_TOL);
    algorithm_formatter.update_bound(lb, "");

    if (!hasIncumbent())
        return;

    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();
    if (!output.solution.feasible()
            || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId vertex_id = 0;
                vertex_id < graph.number_of_vertices();
                ++vertex_id) {
            IloNumArray val_y(y[vertex_id].getEnv());
            IloNumArray val_z(z[vertex_id].getEnv());
            getIncumbentValues(val_y, y[vertex_id]);
            getIncumbentValues(val_z, z[vertex_id]);
            for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
                if (val_y[color_id] + val_z[color_id] < 0.5)
                    solution.set(vertex_id, color_id);
        }
        algorithm_formatter.update_solution(solution, "");
    }
}

const Output coloringsolver::milp_partialordering_cplex(
        const Instance& instance,
        const MilpPartialOrderingCplexParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP - Partial ordering based model (CPLEX)");
    algorithm_formatter.print_header();

    if (instance.adjacency_list_graph() == nullptr) {
        throw std::runtime_error(
                "The 'milp_partialordering_cplex' algorithm requires an AdjacencyListGraph.");
    }
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = graph.highest_degree() + 1;

    // Variables
    // y[v][c] == 1 iff vertex v has color > c.
    // z[v][c] == 1 iff vertex v has color < c.
    std::vector<IloNumVarArray> y;
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id)
        y.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    std::vector<IloNumVarArray> z;
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id)
        z.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));

    // Objective
    IloExpr expr(env);
    expr += 1;
    for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
        expr += y[0][color_id];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // y and z extreme values
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        model.add(y[vertex_id][upper_bound - 1] == 0);
        model.add(z[vertex_id][0]               == 0);
    }

    // If the color of v is > c + 1, then it is > c.
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound - 1; ++color_id)
            model.add(y[vertex_id][color_id + 1] <= y[vertex_id][color_id]);
    }

    // A vertex has a color either > c or < c + 1
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound - 1; ++color_id)
            model.add(y[vertex_id][color_id] + z[vertex_id][color_id + 1] == 1);
    }

    // Prevent assigning the same color to two adjacent vertices.
    for (EdgeId edge_id = 0; edge_id < graph.number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = graph.first_end(edge_id);
        VertexId vertex_id_2 = graph.second_end(edge_id);
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
            model.add(
                    y[vertex_id_1][color_id]
                    + z[vertex_id_1][color_id]
                    + y[vertex_id_2][color_id]
                    + z[vertex_id_2][color_id] >= 1);
    }

    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound - 1; ++color_id)
            model.add(y[0][color_id] - y[vertex_id][color_id] >= 0);
    }

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.timer.remaining_time() != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.timer.remaining_time());

    // Callback
    cplex.use(loggingCallbackPartialOrdering(
                env,
                instance,
                algorithm_formatter,
                output,
                upper_bound,
                y,
                z));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id) {
                for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
                    if (cplex.getValue(y[vertex_id][color_id])
                            + cplex.getValue(z[vertex_id][color_id]) < 0.5) {
                        solution.set(vertex_id, color_id);
                    }
            }
            algorithm_formatter.update_solution(solution, "");
        }
        algorithm_formatter.update_bound(output.solution.number_of_colors(), "");
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id) {
                for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
                    if (cplex.getValue(y[vertex_id][color_id])
                            + cplex.getValue(z[vertex_id][color_id]) < 0.5) {
                        solution.set(vertex_id, color_id);
                    }
            }
            algorithm_formatter.update_solution(solution, "");
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(lb, "");
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(lb, "");
    }

    env.end();

    algorithm_formatter.end();
    return output;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////// Partial-ordering based ILP model 2 //////////////////////
////////////////////////////////////////////////////////////////////////////////

ILOMIPINFOCALLBACK6(loggingCallbackPartialOrdering2,
                    const Instance&, instance,
                    AlgorithmFormatter&, algorithm_formatter,
                    Output&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, y,
                    std::vector<IloNumVarArray>&, z)
{
    ColorId lb = std::ceil(getBestObjValue() - FFOT_TOL);
    algorithm_formatter.update_bound(lb, "");

    if (!hasIncumbent())
        return;

    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();
    if (!output.solution.feasible()
            || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId vertex_id = 0;
                vertex_id < graph.number_of_vertices();
                ++vertex_id) {
            IloNumArray val_y(y[vertex_id].getEnv());
            IloNumArray val_z(z[vertex_id].getEnv());
            getIncumbentValues(val_y, y[vertex_id]);
            getIncumbentValues(val_z, z[vertex_id]);
            for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
                if (val_y[color_id] + val_z[color_id] < 0.5)
                    solution.set(vertex_id, color_id);
        }
        algorithm_formatter.update_solution(solution, "");
    }
}

const Output coloringsolver::milp_partialordering2_cplex(
        const Instance& instance,
        const MilpPartialOrdering2CplexParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP - Partial ordering based model 2 (CPLEX)");
    algorithm_formatter.print_header();

    if (instance.adjacency_list_graph() == nullptr) {
        throw std::runtime_error(
                "The 'milp_partialordering2_cplex' algorithm requires an AdjacencyListGraph.");
    }
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = graph.highest_degree() + 1;

    // Variables
    // x[v][c] == 1 iff vertex v has color c.
    std::vector<IloNumVarArray> x;
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        x.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    }
    // y[v][c] == 1 iff vertex v has color > c.
    // z[v][c] == 1 iff vertex v has color < c.
    std::vector<IloNumVarArray> y;
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        y.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    }
    std::vector<IloNumVarArray> z;
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        z.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    }

    // Objective
    IloExpr expr(env);
    expr += 1;
    for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
        expr += y[0][color_id];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // y and z extreme values
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        model.add(y[vertex_id][upper_bound - 1] == 0);
        model.add(z[vertex_id][0]               == 0);
    }

    // If the color of v is > c + 1, then it is > c.
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound - 1; ++color_id)
            model.add(y[vertex_id][color_id + 1] <= y[vertex_id][color_id]);
    }

    // A vertex has a color either > c or < c + 1
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound - 1; ++color_id)
            model.add(y[vertex_id][color_id] + z[vertex_id][color_id + 1] == 1);
    }

    // Link x with y and z.
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
            model.add(
                    x[vertex_id][color_id]
                    + y[vertex_id][color_id]
                    + z[vertex_id][color_id] == 1);
        }
    }

    // Prevent assigning the same color to two adjacent vertices.
    for (EdgeId edge_id = 0; edge_id < graph.number_of_edges(); ++edge_id) {
        VertexId vertex_id_1 = graph.first_end(edge_id);
        VertexId vertex_id_2 = graph.second_end(edge_id);
        for (ColorId color_id = 0; color_id < upper_bound; ++color_id)
            model.add(x[vertex_id_1][color_id] + x[vertex_id_2][color_id] <= 1);
    }

    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        for (ColorId color_id = 0; color_id < upper_bound - 1; ++color_id)
            model.add(y[0][color_id] - y[vertex_id][color_id] >= 0);
    }

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.timer.remaining_time() != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.timer.remaining_time());

    // Callback
    cplex.use(loggingCallbackPartialOrdering2(
                env,
                instance,
                algorithm_formatter,
                output,
                upper_bound,
                y,
                z));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id) {
                for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
                    if (cplex.getValue(y[vertex_id][color_id])
                            + cplex.getValue(z[vertex_id][color_id]) < 0.5) {
                        solution.set(vertex_id, color_id);
                    }
                }
            }
            algorithm_formatter.update_solution(solution, "");
        }
        algorithm_formatter.update_bound(output.solution.number_of_colors(), "");
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id) {
                for (ColorId color_id = 0; color_id < upper_bound; ++color_id) {
                    if (cplex.getValue(y[vertex_id][color_id])
                            + cplex.getValue(z[vertex_id][color_id]) < 0.5)
                        solution.set(vertex_id, color_id);
                }
            }
            algorithm_formatter.update_solution(solution, "");
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(lb, "");
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(lb, "");
    }

    env.end();

    algorithm_formatter.end();
    return output;
}

#endif
