#if CPLEX_FOUND

#include "coloringsolver/algorithms/milp_cplex.hpp"

#include <ilcplex/ilocplex.h>

using namespace coloringsolver;

ILOSTLBEGIN

/************************* Assignment-based ILP model *************************/

MilpAssignmentCplexOutput& MilpAssignmentCplexOutput::algorithm_end(optimizationtools::Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
    return *this;
}

ILOMIPINFOCALLBACK5(loggingCallbackAssignment,
                    const Instance&, instance,
                    MilpAssignmentCplexOptionalParameters&, parameters,
                    MilpAssignmentCplexOutput&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, x)
{
    ColorId lb = std::ceil(getBestObjValue() - TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible() || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
            IloNumArray val(x[v].getEnv());
            getIncumbentValues(val, x[v]);
            for (ColorId c = 0; c < upper_bound; ++c)
                if (val[c] > 0.5)
                    solution.set(v, c);
        }
        output.update_solution(solution, std::stringstream(""), parameters.info);
    }
}

MilpAssignmentCplexOutput coloringsolver::milp_assignment_cplex(
        const Instance& instance, MilpAssignmentCplexOptionalParameters parameters)
{
    VER(parameters.info, "*** milp_assignment_cplex ***" << std::endl);

    MilpAssignmentCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = instance.maximum_degree() + 1;

    // Variables
    // x[v][c] == 1 iff vertex v has color c.
    std::vector<IloNumVarArray> x;
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
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
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        for (ColorId c = 0; c < upper_bound; ++c) {
            IloExpr expr(env);
            expr += x[instance.edge(e).v1][c];
            expr += x[instance.edge(e).v2][c];
            expr -= y[c];
            model.add(expr <= 0);
        }
    }

    // Each vertex must have a color.
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        IloExpr expr(env);
        for (ColorId c = 0; c < upper_bound; ++c)
            expr += x[v][c];
        model.add(expr == 1);
    }

    if (parameters.break_symmetries) {
        for (ColorId c = 0; c < upper_bound; ++c) {
            IloExpr expr(env);
            expr += y[c];
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
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
    cplex.use(loggingCallbackAssignment(env, instance, parameters, output, upper_bound, x));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible() || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(x[v][c]) > 0.5)
                        solution.set(v, c);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        output.update_lower_bound(output.solution.number_of_colors(), std::stringstream(""), parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible() || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(x[v][c]) > 0.5)
                        solution.set(v, c);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

/************************* Representatives ILP model **************************/

MilpRepresentativesCplexOutput& MilpRepresentativesCplexOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
    return *this;
}

ILOMIPINFOCALLBACK5(loggingCallbackRepresentatives,
                    const Instance&, instance,
                    MilpRepresentativesCplexOptionalParameters&, parameters,
                    MilpRepresentativesCplexOutput&, output,
                    const std::vector<Edge>&, complementary_edges,
                    std::vector<IloNumVarArray>&, x)
{
    ColorId lb = std::ceil(getBestObjValue() - TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible() || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v1 = 0; v1 < instance.number_of_vertices(); ++v1) {
            for (VertexId v2 = 0; v2 < instance.number_of_vertices(); ++v2) {
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
        const Instance& instance, MilpRepresentativesCplexOptionalParameters parameters)
{
    VER(parameters.info, "*** milp_representatives_cplex ***" << std::endl);

    MilpRepresentativesCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    // Build complementary graph.
    std::vector<Edge> complementary_edges;
    optimizationtools::IndexedSet neighbors(instance.number_of_vertices());
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        neighbors.clear();
        for (const auto& vn: instance.vertex(v).edges)
            neighbors.add(vn.v);
        for (auto it = neighbors.out_begin(); it != neighbors.out_end(); ++it)
            if (instance.degree(v) > instance.degree(*it)
                    || (instance.degree(v) == instance.degree(*it) && v < *it))
                complementary_edges.push_back({(EdgeId)complementary_edges.size(), v, *it});
    }
    std::vector<std::vector<VertexNeighbor>> complementary_graph(instance.number_of_vertices());
    for (EdgeId e = 0; e < (EdgeId)complementary_edges.size(); ++e) {
        const Edge& edge = complementary_edges[e];
        VertexNeighbor vn;
        vn.e = e;
        vn.v = edge.v1;
        complementary_graph[edge.v2].push_back(vn);
    }

    // Variables
    // x[u][v] == 1 iff vertex v is represented by u.
    std::vector<IloNumVarArray> x(instance.number_of_vertices());
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        x[v] = IloNumVarArray(env, instance.number_of_vertices(), 0, 1, ILOBOOL);

    // Objective
    IloExpr expr(env);
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        expr += x[v][v];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // Every vertex v must have a representative (possibly v itself).
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        IloExpr expr(env);
        expr += x[v][v];
        for (const auto& vn: complementary_graph[v])
            expr += x[vn.v][v];
        model.add(expr >= 1);
    }

    // A representative can not represent both endpoints of an edge.
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        const Edge& edge = instance.edge(e);
        for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
            model.add(x[v][edge.v1] + x[v][edge.v2] <= x[v][v]);
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
    cplex.use(loggingCallbackRepresentatives(env, instance, parameters, output, complementary_edges, x));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible() || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v1 = 0; v1 < instance.number_of_vertices(); ++v1) {
                for (VertexId v2 = 0; v2 < instance.number_of_vertices(); ++v2) {
                    if (cplex.getValue(x[v2][v1]) > 0.5) {
                        solution.set(v1, v2);
                        break;
                    }
                }
            }
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        output.update_lower_bound(output.solution.number_of_colors(), std::stringstream(""), parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible() || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v1 = 0; v1 < instance.number_of_vertices(); ++v1) {
                for (VertexId v2 = 0; v2 < instance.number_of_vertices(); ++v2) {
                    if (cplex.getValue(x[v2][v1]) > 0.5) {
                        solution.set(v1, v2);
                        break;
                    }
                }
            }
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

/********************** Partial-ordering based ILP model **********************/

MilpPartialOrderingCplexOutput& MilpPartialOrderingCplexOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
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
    ColorId lb = std::ceil(getBestObjValue() - TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible() || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
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
        const Instance& instance, MilpPartialOrderingCplexOptionalParameters parameters)
{
    VER(parameters.info, "*** milp_partialordering_cplex ***" << std::endl);

    MilpPartialOrderingCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = instance.maximum_degree() + 1;

    // Variables
    // y[v][c] == 1 iff vertex v has color > c.
    // z[v][c] == 1 iff vertex v has color < c.
    std::vector<IloNumVarArray> y;
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        y.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    std::vector<IloNumVarArray> z;
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        z.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));

    // Objective
    IloExpr expr(env);
    expr += 1;
    for (ColorId c = 0; c < upper_bound; ++c)
        expr += y[0][c];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // y and z extreme values
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        model.add(y[v][upper_bound - 1] == 0);
        model.add(z[v][0]               == 0);
    }

    // If the color of v is > c + 1, then it is > c.
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c + 1] <= y[v][c]);

    // A vertex has a color either > c or < c + 1
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c] + z[v][c + 1] == 1);

    // Prevent assigning the same color to two adjacent vertices.
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        const Edge& edge = instance.edge(e);
        for (ColorId c = 0; c < upper_bound; ++c)
            model.add(y[edge.v1][c] + z[edge.v1][c] + y[edge.v2][c] + z[edge.v2][c] >= 1);
    }

    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
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
    cplex.use(loggingCallbackPartialOrdering(env, instance, parameters, output, upper_bound, y, z));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible() || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(y[v][c]) + cplex.getValue(z[v][c]) < 0.5)
                        solution.set(v, c);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        output.update_lower_bound(output.solution.number_of_colors(), std::stringstream(""), parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible() || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(y[v][c]) + cplex.getValue(z[v][c]) < 0.5)
                        solution.set(v, c);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

/********************* Partial-ordering based ILP model 2 *********************/

MilpPartialOrdering2CplexOutput& MilpPartialOrdering2CplexOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
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
    ColorId lb = std::ceil(getBestObjValue() - TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible() || output.solution.number_of_colors() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
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

MilpPartialOrdering2CplexOutput coloringsolver::milp_partialordering2_cplex(
        const Instance& instance, MilpPartialOrdering2CplexOptionalParameters parameters)
{
    VER(parameters.info, "*** milp_partialordering2_cplex ***" << std::endl);

    MilpPartialOrdering2CplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = instance.maximum_degree() + 1;

    // Variables
    // x[v][c] == 1 iff vertex v has color c.
    std::vector<IloNumVarArray> x;
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        x.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    // y[v][c] == 1 iff vertex v has color > c.
    // z[v][c] == 1 iff vertex v has color < c.
    std::vector<IloNumVarArray> y;
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        y.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    std::vector<IloNumVarArray> z;
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        z.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));

    // Objective
    IloExpr expr(env);
    expr += 1;
    for (ColorId c = 0; c < upper_bound; ++c)
        expr += y[0][c];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // y and z extreme values
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
        model.add(y[v][upper_bound - 1] == 0);
        model.add(z[v][0]               == 0);
    }

    // If the color of v is > c + 1, then it is > c.
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c + 1] <= y[v][c]);

    // A vertex has a color either > c or < c + 1
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c] + z[v][c + 1] == 1);

    // Link x with y and z.
    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
        for (ColorId c = 0; c < upper_bound; ++c)
            model.add(x[v][c] + y[v][c] + z[v][c] == 1);

    // Prevent assigning the same color to two adjacent vertices.
    for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
        const Edge& edge = instance.edge(e);
        for (ColorId c = 0; c < upper_bound; ++c)
            model.add(x[edge.v1][c] + x[edge.v2][c] <= 1);
    }

    for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
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
    cplex.use(loggingCallbackPartialOrdering2(env, instance, parameters, output, upper_bound, y, z));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible() || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(y[v][c]) + cplex.getValue(z[v][c]) < 0.5)
                        solution.set(v, c);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        output.update_lower_bound(output.solution.number_of_colors(), std::stringstream(""), parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible() || output.solution.number_of_colors() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(y[v][c]) + cplex.getValue(z[v][c]) < 0.5)
                        solution.set(v, c);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    }

    env.end();

    return output.algorithm_end(parameters.info);
}

#endif

