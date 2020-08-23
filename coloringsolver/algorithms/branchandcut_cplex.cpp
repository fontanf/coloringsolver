#if CPLEX_FOUND

#include "coloringsolver/algorithms/branchandcut_cplex.hpp"

#include <ilcplex/ilocplex.h>

using namespace coloringsolver;

ILOSTLBEGIN

/************************* Assignment-based ILP model *************************/

BranchAndCutAssignmentCplexOutput& BranchAndCutAssignmentCplexOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
    return *this;
}

ILOMIPINFOCALLBACK5(loggingCallbackAssignment,
                    const Instance&, instance,
                    BranchAndCutAssignmentCplexOptionalParameters&, parameters,
                    BranchAndCutAssignmentCplexOutput&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, x)
{
    ColorId lb = std::ceil(getBestObjValue() - TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible() || output.solution.color_number() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v = 0; v < instance.vertex_number(); ++v) {
            IloNumArray val(x[v].getEnv());
            getIncumbentValues(val, x[v]);
            for (ColorId c = 0; c < upper_bound; ++c)
                if (val[c] > 0.5)
                    solution.set(v, c);
        }
        output.update_solution(solution, std::stringstream(""), parameters.info);
    }
}

BranchAndCutAssignmentCplexOutput coloringsolver::branchandcut_assignment_cplex(
        const Instance& instance, BranchAndCutAssignmentCplexOptionalParameters parameters)
{
    VER(parameters.info, "*** branchandcut_assignment_cplex ***" << std::endl);

    BranchAndCutAssignmentCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = instance.degree_max() + 1;

    // Variables
    // x[v][c] == 1 iff vertex v has color c.
    std::vector<IloNumVarArray> x;
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
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
    for (EdgeId e = 0; e < instance.edge_number(); ++e) {
        for (ColorId c = 0; c < upper_bound; ++c) {
            IloExpr expr(env);
            expr += x[instance.edge(e).v1][c];
            expr += x[instance.edge(e).v2][c];
            expr -= y[c];
            model.add(expr <= 0);
        }
    }

    // Each vertex must have a color.
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        IloExpr expr(env);
        for (ColorId c = 0; c < upper_bound; ++c)
            expr += x[v][c];
        model.add(expr == 1);
    }

    if (parameters.break_symmetries) {
        for (ColorId c = 0; c < upper_bound; ++c) {
            IloExpr expr(env);
            expr += y[c];
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
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
    if (parameters.info.timelimit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.info.remaining_time());

    // Callback
    cplex.use(loggingCallbackAssignment(env, instance, parameters, output, upper_bound, x));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible() || output.solution.color_number() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(x[v][c]) > 0.5)
                        solution.set(v, c);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        output.update_lower_bound(output.solution.color_number(), std::stringstream(""), parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible() || output.solution.color_number() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
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

BranchAndCutRepresentativesCplexOutput& BranchAndCutRepresentativesCplexOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
    return *this;
}

ILOMIPINFOCALLBACK5(loggingCallbackRepresentatives,
                    const Instance&, instance,
                    BranchAndCutRepresentativesCplexOptionalParameters&, parameters,
                    BranchAndCutRepresentativesCplexOutput&, output,
                    const std::vector<Edge>&, complementary_edges,
                    std::vector<IloNumVarArray>&, x)
{
    ColorId lb = std::ceil(getBestObjValue() - TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible() || output.solution.color_number() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v1 = 0; v1 < instance.vertex_number(); ++v1) {
            for (VertexId v2 = 0; v2 < instance.vertex_number(); ++v2) {
                if (getIncumbentValue(x[v2][v1]) > 0.5) {
                    solution.set(v1, v2);
                    break;
                }
            }
        }
        output.update_solution(solution, std::stringstream(""), parameters.info);
    }
}

BranchAndCutRepresentativesCplexOutput coloringsolver::branchandcut_representatives_cplex(
        const Instance& instance, BranchAndCutRepresentativesCplexOptionalParameters parameters)
{
    VER(parameters.info, "*** branchandcut_representatives_cplex --break-symmetries " << parameters.break_symmetries << " ***" << std::endl);

    BranchAndCutRepresentativesCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    // Build complementary graph.
    std::vector<Edge> complementary_edges;
    optimizationtools::IndexedSet neighbors(instance.vertex_number());
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        neighbors.clear();
        for (const auto& vn: instance.vertex(v).edges)
            neighbors.add(vn.v);
        for (auto it = neighbors.out_begin(); it != neighbors.out_end(); ++it) {
            if (*it >= v)
                continue;
            complementary_edges.push_back({(EdgeId)complementary_edges.size(), v, *it});
        }
    }
    std::vector<std::vector<VertexNeighbor>> complementary_graph(instance.vertex_number());
    for (EdgeId e = 0; e < (EdgeId)complementary_edges.size(); ++e) {
        const Edge& edge = complementary_edges[e];
        VertexNeighbor vn1;
        vn1.e = e;
        vn1.v = edge.v2;
        complementary_graph[edge.v1].push_back(vn1);
        VertexNeighbor vn2;
        vn2.e = e;
        vn2.v = edge.v1;
        complementary_graph[edge.v2].push_back(vn2);
    }

    // Variables
    // x[u][v] == 1 iff vertex v is represented by u.
    std::vector<IloNumVarArray> x(instance.vertex_number());
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        x[v] = IloNumVarArray(env, instance.vertex_number(), 0, 1, ILOBOOL);

    // Objective
    IloExpr expr(env);
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        expr += x[v][v];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // Every vertex v must have a representative (possibly v itself).
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        IloExpr expr(env);
        expr += x[v][v];
        for (const auto& vn: complementary_graph[v])
            if (!parameters.break_symmetries || vn.v < v)
                expr += x[vn.v][v];
        model.add(expr >= 1);
    }

    // A representative can not represent both endpoints of an edge.
    for (EdgeId e = 0; e < instance.edge_number(); ++e) {
        const Edge& edge = instance.edge(e);
        for (VertexId v = 0; v < instance.vertex_number(); ++v)
            model.add(x[v][edge.v1] + x[v][edge.v2] <= x[v][v]);
    }

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.info.timelimit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.info.remaining_time());

    // Callback
    cplex.use(loggingCallbackRepresentatives(env, instance, parameters, output, complementary_edges, x));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible() || output.solution.color_number() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v1 = 0; v1 < instance.vertex_number(); ++v1) {
                for (VertexId v2 = 0; v2 < instance.vertex_number(); ++v2) {
                    if (cplex.getValue(x[v2][v1]) > 0.5) {
                        solution.set(v1, v2);
                        break;
                    }
                }
            }
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        output.update_lower_bound(output.solution.color_number(), std::stringstream(""), parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible() || output.solution.color_number() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v1 = 0; v1 < instance.vertex_number(); ++v1) {
                for (VertexId v2 = 0; v2 < instance.vertex_number(); ++v2) {
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

BranchAndCutPartialOrderingCplexOutput& BranchAndCutPartialOrderingCplexOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
    return *this;
}

ILOMIPINFOCALLBACK6(loggingCallbackPartialOrdering,
                    const Instance&, instance,
                    BranchAndCutPartialOrderingCplexOptionalParameters&, parameters,
                    BranchAndCutPartialOrderingCplexOutput&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, y,
                    std::vector<IloNumVarArray>&, z)
{
    ColorId lb = std::ceil(getBestObjValue() - TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible() || output.solution.color_number() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v = 0; v < instance.vertex_number(); ++v) {
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

BranchAndCutPartialOrderingCplexOutput coloringsolver::branchandcut_partialordering_cplex(
        const Instance& instance, BranchAndCutPartialOrderingCplexOptionalParameters parameters)
{
    VER(parameters.info, "*** branchandcut_partialordering_cplex ***" << std::endl);

    BranchAndCutPartialOrderingCplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = instance.degree_max() + 1;

    // Variables
    // y[v][c] == 1 iff vertex v has color > c.
    // z[v][c] == 1 iff vertex v has color < c.
    std::vector<IloNumVarArray> y;
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        y.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    std::vector<IloNumVarArray> z;
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        z.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));

    // Objective
    IloExpr expr(env);
    expr += 1;
    for (ColorId c = 0; c < upper_bound; ++c)
        expr += y[0][c];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // y and z extreme values
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        model.add(y[v][upper_bound - 1] == 0);
        model.add(z[v][0]               == 0);
    }

    // If the color of v is > c + 1, then it is > c.
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c + 1] <= y[v][c]);

    // A vertex has a color either > c or < c + 1
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c] + z[v][c + 1] == 1);

    // Prevent assigning the same color to two adjacent vertices.
    for (EdgeId e = 0; e < instance.edge_number(); ++e) {
        const Edge& edge = instance.edge(e);
        for (ColorId c = 0; c < upper_bound; ++c)
            model.add(y[edge.v1][c] + z[edge.v1][c] + y[edge.v2][c] + z[edge.v2][c] >= 1);
    }

    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[0][c] - y[v][c] >= 0);

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.info.timelimit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.info.remaining_time());

    // Callback
    cplex.use(loggingCallbackPartialOrdering(env, instance, parameters, output, upper_bound, y, z));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible() || output.solution.color_number() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(y[v][c]) + cplex.getValue(z[v][c]) < 0.5)
                        solution.set(v, c);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        output.update_lower_bound(output.solution.color_number(), std::stringstream(""), parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible() || output.solution.color_number() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
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

BranchAndCutPartialOrdering2CplexOutput& BranchAndCutPartialOrdering2CplexOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
    return *this;
}

ILOMIPINFOCALLBACK6(loggingCallbackPartialOrdering2,
                    const Instance&, instance,
                    BranchAndCutPartialOrdering2CplexOptionalParameters&, parameters,
                    BranchAndCutPartialOrdering2CplexOutput&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, y,
                    std::vector<IloNumVarArray>&, z)
{
    ColorId lb = std::ceil(getBestObjValue() - TOL);
    output.update_lower_bound(lb, std::stringstream(""), parameters.info);

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible() || output.solution.color_number() > getIncumbentObjValue() + 0.5) {
        Solution solution(instance);
        for (VertexId v = 0; v < instance.vertex_number(); ++v) {
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

BranchAndCutPartialOrdering2CplexOutput coloringsolver::branchandcut_partialordering2_cplex(
        const Instance& instance, BranchAndCutPartialOrdering2CplexOptionalParameters parameters)
{
    VER(parameters.info, "*** branchandcut_partialordering2_cplex ***" << std::endl);

    BranchAndCutPartialOrdering2CplexOutput output(instance, parameters.info);

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = instance.degree_max() + 1;

    // Variables
    // x[v][c] == 1 iff vertex v has color c.
    std::vector<IloNumVarArray> x;
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        x.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    // y[v][c] == 1 iff vertex v has color > c.
    // z[v][c] == 1 iff vertex v has color < c.
    std::vector<IloNumVarArray> y;
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        y.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    std::vector<IloNumVarArray> z;
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        z.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));

    // Objective
    IloExpr expr(env);
    expr += 1;
    for (ColorId c = 0; c < upper_bound; ++c)
        expr += y[0][c];
    IloObjective obj = IloMinimize(env, expr);
    model.add(obj);

    // y and z extreme values
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        model.add(y[v][upper_bound - 1] == 0);
        model.add(z[v][0]               == 0);
    }

    // If the color of v is > c + 1, then it is > c.
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c + 1] <= y[v][c]);

    // A vertex has a color either > c or < c + 1
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[v][c] + z[v][c + 1] == 1);

    // Link x with y and z.
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        for (ColorId c = 0; c < upper_bound; ++c)
            model.add(x[v][c] + y[v][c] + z[v][c] == 1);

    // Prevent assigning the same color to two adjacent vertices.
    for (EdgeId e = 0; e < instance.edge_number(); ++e) {
        const Edge& edge = instance.edge(e);
        for (ColorId c = 0; c < upper_bound; ++c)
            model.add(x[edge.v1][c] + x[edge.v2][c] <= 1);
    }

    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        for (ColorId c = 0; c < upper_bound - 1; ++c)
            model.add(y[0][c] - y[v][c] >= 0);

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (parameters.info.timelimit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, parameters.info.remaining_time());

    // Callback
    cplex.use(loggingCallbackPartialOrdering2(env, instance, parameters, output, upper_bound, y, z));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible() || output.solution.color_number() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(y[v][c]) + cplex.getValue(z[v][c]) < 0.5)
                        solution.set(v, c);
            output.update_solution(solution, std::stringstream(""), parameters.info);
        }
        output.update_lower_bound(output.solution.color_number(), std::stringstream(""), parameters.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible() || output.solution.color_number() > cplex.getObjValue() + 0.5) {
            Solution solution(instance);
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
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

