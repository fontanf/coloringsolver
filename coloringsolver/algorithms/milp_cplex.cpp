#if CPLEX_FOUND

#include "coloringsolver/algorithms/milp_cplex.hpp"

#include <ilcplex/ilocplex.h>

using namespace coloringsolver;

ILOSTLBEGIN

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Assignment-based ILP model //////////////////////////
////////////////////////////////////////////////////////////////////////////////

MilpAssignmentCplexOutput& MilpAssignmentCplexOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //info.add_to_json("Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //info.os() << "Iterations: " << it << std::endl;
    return *this;
}

ILOMIPINFOCALLBACK5(loggingCallbackAssignment,
                    const Instance&, instance,
                    MilpAssignmentCplexOptionalParameters&, parameters,
                    MilpAssignmentCplexOutput&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, x)
{
    ColorId lb = std::ceil(getBestObjValue() - FFOT_TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible()
            || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v = 0; v < instance.graph().number_of_vertices(); ++v) {
            IloNumArray val(x[v].getEnv());
            getIncumbentValues(val, x[v]);
            for (ColorId c = 0; c < upper_bound; ++c)
                if (val[c] > 0.5)
                    solution.set(v, c);
        }
        output.update_solution(
                solution,
                std::stringstream(""),
                parameters.info);
    }
}

MilpAssignmentCplexOutput coloringsolver::milp_assignment_cplex(
        const Instance& instance,
        MilpAssignmentCplexOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "MILP - Assignment model (CPLEX)" << std::endl
        << std::endl;

    if (instance.adjacency_list_graph() == nullptr) {
        throw std::runtime_error(
                "The 'milp_assignment_cplex' algorithm requires an AdjacencyListGraph.");
    }
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    MilpAssignmentCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = graph.maximum_degree() + 1;

    // Variables
    // x[v][c] == 1 iff vertex v has color c.
    std::vector<IloNumVarArray> x;
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        x.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    // y[c] == 1 iff color c is used.
    IloNumVarArray y(env, upper_bound, 0, 1, ILOBOOL);

    // Objective
    IloExpr expr(env);
    for (ColorId c = 0; c < upper_bound; ++ c)
        expr += y[c];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // Edge constraints
    for (EdgeId e = 0; e < graph.number_of_edges(); ++e) {
        for (ColorId c = 0; c < upper_bound; ++c) {
            IloExpr expr(env);
            expr += x[graph.first_end(e)][c];
            expr += x[graph.second_end(e)][c];
            expr -= y[c];
            model.add(expr <= 0);
        }
    }

    // Each vertex must have a color.
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v) {
        IloExpr expr(env);
        for (ColorId c = 0; c < upper_bound; ++c)
            expr += x[v][c];
        model.add(expr == 1);
    }

    if (parameters.break_symmetries) {
        for (ColorId c = 0; c < upper_bound; ++c) {
            IloExpr expr(env);
            expr += y[c];
            for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
                expr -= x[v][c];
            model.add(expr <= 0);
        }
        for (ColorId c = 1; c < upper_bound; ++c)
            model.add(y[c] <= y[c - 1]);
    }

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.info.time_limit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.info.remaining_time());

    // Callback
    cplex.use(loggingCallbackAssignment(
                env,
                instance,
                parameters,
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
            for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(x[v][c]) > 0.5)
                        solution.set(v, c);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        output.update_lower_bound(
                output.solution.number_of_colors(),
                std::stringstream(""),
                parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(x[v][c]) > 0.5)
                        solution.set(v, c);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////// Representatives ILP model ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

MilpRepresentativesCplexOutput& MilpRepresentativesCplexOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //info.add_to_json("Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //info.os() "Iterations: " << it << std::endl;
    return *this;
}

ILOMIPINFOCALLBACK5(loggingCallbackRepresentatives,
                    const Instance&, instance,
                    MilpRepresentativesCplexOptionalParameters&, parameters,
                    MilpRepresentativesCplexOutput&, output,
                    const optimizationtools::AdjacencyListGraph&, complementary_graph,
                    std::vector<IloNumVarArray>&, x)
{
    ColorId lb = std::ceil(getBestObjValue() - FFOT_TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();
    if (!output.solution.feasible()
            || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v1 = 0; v1 < graph.number_of_vertices(); ++v1) {
            for (VertexId v2 = 0; v2 < graph.number_of_vertices(); ++v2) {
                if (getIncumbentValue(x[v2][v1]) > 0.5) {
                    solution.set(v1, v2);
                    break;
                }
            }
        }
        output.update_solution(solution, std::stringstream(""), parameters.info);
    }
}

MilpRepresentativesCplexOutput coloringsolver::milp_representatives_cplex(
        const Instance& instance,
        MilpRepresentativesCplexOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "MILP - Representative model (CPLEX)" << std::endl
        << std::endl;

    if (instance.adjacency_list_graph() == nullptr) {
        throw std::runtime_error(
                "The 'milp_representatives_cplex' algorithm requires an AdjacencyListGraph.");
    }
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    MilpRepresentativesCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    // Build complementary graph.
    optimizationtools::AdjacencyListGraph complementary_graph = graph.complementary();

    // Variables
    // x[u][v] == 1 iff vertex v is represented by u.
    std::vector<IloNumVarArray> x(graph.number_of_vertices());
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        x[v] = IloNumVarArray(env, graph.number_of_vertices(), 0, 1, ILOBOOL);

    // Objective
    IloExpr expr(env);
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        expr += x[v][v];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // Every vertex v must have a representative (possibly v itself).
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v) {
        IloExpr expr(env);
        expr += x[v][v];
        for (auto it = complementary_graph.neighbors_begin(v);
                it != complementary_graph.neighbors_end(v); ++it) {
            expr += x[*it][v];
        }
        model.add(expr >= 1);
    }

    // A representative can not represent both endpoints of an edge.
    for (EdgeId e = 0; e < graph.number_of_edges(); ++e) {
        for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
            model.add(x[v][graph.first_end(e)] + x[v][graph.second_end(e)] <= x[v][v]);
    }

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.info.time_limit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.info.remaining_time());

    // Callback
    cplex.use(loggingCallbackRepresentatives(
                env,
                instance,
                parameters,
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
            for (VertexId v1 = 0; v1 < graph.number_of_vertices(); ++v1) {
                for (VertexId v2 = 0; v2 < graph.number_of_vertices(); ++v2) {
                    if (cplex.getValue(x[v2][v1]) > 0.5) {
                        solution.set(v1, v2);
                        break;
                    }
                }
            }
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        output.update_lower_bound(
                output.solution.number_of_colors(),
                std::stringstream(""),
                parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v1 = 0; v1 < graph.number_of_vertices(); ++v1) {
                for (VertexId v2 = 0; v2 < graph.number_of_vertices(); ++v2) {
                    if (cplex.getValue(x[v2][v1]) > 0.5) {
                        solution.set(v1, v2);
                        break;
                    }
                }
            }
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Partial-ordering based ILP model ///////////////////////
////////////////////////////////////////////////////////////////////////////////

MilpPartialOrderingCplexOutput& MilpPartialOrderingCplexOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //info.add_to_json("Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //info.os() << "Iterations: " << it << std::endl;
    return *this;
}

ILOMIPINFOCALLBACK6(loggingCallbackPartialOrdering,
                    const Instance&, instance,
                    MilpPartialOrderingCplexOptionalParameters&, parameters,
                    MilpPartialOrderingCplexOutput&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, y,
                    std::vector<IloNumVarArray>&, z)
{
    ColorId lb = std::ceil(getBestObjValue() - FFOT_TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();
    if (!output.solution.feasible()
            || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v = 0; v < graph.number_of_vertices(); ++v) {
            IloNumArray val_y(y[v].getEnv());
            IloNumArray val_z(z[v].getEnv());
            getIncumbentValues(val_y, y[v]);
            getIncumbentValues(val_z, z[v]);
            for (ColorId c = 0; c < upper_bound; ++c)
                if (val_y[c] + val_z[c] < 0.5)
                    solution.set(v, c);
        }
        output.update_solution(solution, std::stringstream(""), parameters.info);
    }
}

MilpPartialOrderingCplexOutput coloringsolver::milp_partialordering_cplex(
        const Instance& instance,
        MilpPartialOrderingCplexOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "MILP - Partial ordering based model (CPLEX)" << std::endl
        << std::endl;

    if (instance.adjacency_list_graph() == nullptr) {
        throw std::runtime_error(
                "The 'milp_partialordering_cplex' algorithm requires an AdjacencyListGraph.");
    }
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    MilpPartialOrderingCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = graph.maximum_degree() + 1;

    // Variables
    // y[v][c] == 1 iff vertex v has color > c.
    // z[v][c] == 1 iff vertex v has color < c.
    std::vector<IloNumVarArray> y;
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        y.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    std::vector<IloNumVarArray> z;
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        z.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));

    // Objective
    IloExpr expr(env);
    expr += 1;
    for (ColorId c = 0; c < upper_bound; ++c)
        expr += y[0][c];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // y and z extreme values
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v) {
        model.add(y[v][upper_bound - 1] == 0);
        model.add(z[v][0]               == 0);
    }

    // If the color of v is > c + 1, then it is > c.
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c + 1] <= y[v][c]);

    // A vertex has a color either > c or < c + 1
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c] + z[v][c + 1] == 1);

    // Prevent assigning the same color to two adjacent vertices.
    for (EdgeId e = 0; e < graph.number_of_edges(); ++e) {
        VertexId v1 = graph.first_end(e);
        VertexId v2 = graph.second_end(e);
        for (ColorId c = 0; c < upper_bound; ++c)
            model.add(y[v1][c] + z[v1][c] + y[v2][c] + z[v2][c] >= 1);
    }

    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[0][c] - y[v][c] >= 0);

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.info.time_limit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.info.remaining_time());

    // Callback
    cplex.use(loggingCallbackPartialOrdering(
                env,
                instance,
                parameters,
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
            for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(y[v][c]) + cplex.getValue(z[v][c]) < 0.5)
                        solution.set(v, c);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        output.update_lower_bound(
                output.solution.number_of_colors(),
                std::stringstream(""),
                parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(y[v][c]) + cplex.getValue(z[v][c]) < 0.5)
                        solution.set(v, c);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////// Partial-ordering based ILP model 2 //////////////////////
////////////////////////////////////////////////////////////////////////////////

MilpPartialOrdering2CplexOutput& MilpPartialOrdering2CplexOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //info.add_to_json("Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //info.os() << "Iterations: " << it << std::endl;
    return *this;
}

ILOMIPINFOCALLBACK6(loggingCallbackPartialOrdering2,
                    const Instance&, instance,
                    MilpPartialOrdering2CplexOptionalParameters&, parameters,
                    MilpPartialOrdering2CplexOutput&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, y,
                    std::vector<IloNumVarArray>&, z)
{
    ColorId lb = std::ceil(getBestObjValue() - FFOT_TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();
    if (!output.solution.feasible()
            || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v = 0; v < graph.number_of_vertices(); ++v) {
            IloNumArray val_y(y[v].getEnv());
            IloNumArray val_z(z[v].getEnv());
            getIncumbentValues(val_y, y[v]);
            getIncumbentValues(val_z, z[v]);
            for (ColorId c = 0; c < upper_bound; ++c)
                if (val_y[c] + val_z[c] < 0.5)
                    solution.set(v, c);
        }
        output.update_solution(
                solution,
                std::stringstream(""),
                parameters.info);
    }
}

MilpPartialOrdering2CplexOutput coloringsolver::milp_partialordering2_cplex(
        const Instance& instance,
        MilpPartialOrdering2CplexOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "MILP - Partial ordering based model 2 (CPLEX)" << std::endl
        << std::endl;

    if (instance.adjacency_list_graph() == nullptr) {
        throw std::runtime_error(
                "The 'milp_partialordering2_cplex' algorithm requires an AdjacencyListGraph.");
    }
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    MilpPartialOrdering2CplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = graph.maximum_degree() + 1;

    // Variables
    // x[v][c] == 1 iff vertex v has color c.
    std::vector<IloNumVarArray> x;
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        x.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    // y[v][c] == 1 iff vertex v has color > c.
    // z[v][c] == 1 iff vertex v has color < c.
    std::vector<IloNumVarArray> y;
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        y.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    std::vector<IloNumVarArray> z;
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        z.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));

    // Objective
    IloExpr expr(env);
    expr += 1;
    for (ColorId c = 0; c < upper_bound; ++c)
        expr += y[0][c];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // y and z extreme values
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v) {
        model.add(y[v][upper_bound - 1] == 0);
        model.add(z[v][0]               == 0);
    }

    // If the color of v is > c + 1, then it is > c.
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c + 1] <= y[v][c]);

    // A vertex has a color either > c or < c + 1
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c] + z[v][c + 1] == 1);

    // Link x with y and z.
    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound; ++c)
            model.add(x[v][c] + y[v][c] + z[v][c] == 1);

    // Prevent assigning the same color to two adjacent vertices.
    for (EdgeId e = 0; e < graph.number_of_edges(); ++e) {
        VertexId v1 = graph.first_end(e);
        VertexId v2 = graph.second_end(e);
        for (ColorId c = 0; c < upper_bound; ++c)
            model.add(x[v1][c] + x[v2][c] <= 1);
    }

    for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[0][c] - y[v][c] >= 0);

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.info.time_limit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.info.remaining_time());

    // Callback
    cplex.use(loggingCallbackPartialOrdering2(
                env,
                instance,
                parameters,
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
            for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(y[v][c]) + cplex.getValue(z[v][c]) < 0.5)
                        solution.set(v, c);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        output.update_lower_bound(
                output.solution.number_of_colors(),
                std::stringstream(""),
                parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible()
                || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < graph.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(y[v][c]) + cplex.getValue(z[v][c]) < 0.5)
                        solution.set(v, c);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

#endif

