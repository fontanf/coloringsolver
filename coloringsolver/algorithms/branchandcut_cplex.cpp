#if CPLEX_FOUND

#include "coloringsolver/algorithms/branchandcut_cplex.hpp"

#include <ilcplex/ilocplex.h>

using namespace coloringsolver;

ILOSTLBEGIN

BranchAndCutCplexOutput& BranchAndCutCplexOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
    return *this;
}

ILOMIPINFOCALLBACK5(loggingCallback,
                    const Instance&, instance,
                    BranchAndCutCplexOptionalParameters&, p,
                    BranchAndCutCplexOutput&, output,
                    ColorId, upper_bound,
                    std::vector<IloNumVarArray>&, vars)
{
    ColorId lb = std::ceil(getBestObjValue() - TOL);
    output.update_lower_bound(lb, std::stringstream(""), p.info);

    if (!hasIncumbent())
        return;

    if (!output.solution.feasible() || output.solution.color_number() > getIncumbentObjValue() + 0.5) {
        Solution sol_curr(instance);
        for (VertexId v = 0; v < instance.vertex_number(); ++v) {
            IloNumArray val(vars[v].getEnv());
            getIncumbentValues(val, vars[v]);
            for (ColorId c = 0; c < upper_bound; ++c)
                if (val[c] > 0.5)
                    sol_curr.set(v, c);
        }
        output.update_solution(sol_curr, std::stringstream(""), p.info);
    }
}

BranchAndCutCplexOutput coloringsolver::branchandcut_cplex(
        const Instance& instance, BranchAndCutCplexOptionalParameters p)
{
    VER(p.info, "*** branchandcut_cplex ***" << std::endl);

    BranchAndCutCplexOutput output(instance, p.info);

    IloEnv env;
    IloModel model(env);

    ColorId upper_bound = instance.degree_max() + 1;

    // Variables
    // x[v][c] == 1 iff vertex v has color c.
    std::vector<IloNumVarArray> x;
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        x.push_back(IloNumVarArray(env, upper_bound, 0, 1, ILOBOOL));
    // y[c] == 1 iff at least c colors are used.
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

    IloCplex cplex(model);

    // Redirect standard output to log file
    std::ofstream logfile("cplex.log");
    cplex.setOut(logfile);

    cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.0); // Fix precision issue
    cplex.setParam(IloCplex::Param::MIP::Strategy::File, 2); // Avoid running out of memory

    // Time limit
    if (p.info.timelimit != std::numeric_limits<double>::infinity())
        cplex.setParam(IloCplex::TiLim, p.info.remaining_time());

    // Callback
    cplex.use(loggingCallback(env, instance, p, output, upper_bound, x));

    // Optimize
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
        if (!output.solution.feasible() || output.solution.color_number() > cplex.getObjValue() + 0.5) {
            Solution sol_curr(instance);
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(x[v][c]) > 0.5)
                        sol_curr.set(v, c);
            output.update_solution(sol_curr, std::stringstream(""), p.info);
        }
        output.update_lower_bound(output.solution.color_number(), std::stringstream(""), p.info);
    } else if (cplex.isPrimalFeasible()) {
        if (!output.solution.feasible() || output.solution.color_number() > cplex.getObjValue() + 0.5) {
            Solution sol_curr(instance);
            for (VertexId v = 0; v < instance.vertex_number(); ++v)
                for (ColorId c = 0; c < upper_bound; ++c)
                    if (cplex.getValue(x[v][c]) > 0.5)
                        sol_curr.set(v, c);
            output.update_solution(sol_curr, std::stringstream(""), p.info);
        }
        ColorId lb = std::ceil(cplex.getBestObjValue() - TOL);
        output.update_lower_bound(lb, std::stringstream(""), p.info);
    } else {
        ColorId lb = std::ceil(cplex.getBestObjValue() - TOL);
        output.update_lower_bound(lb, std::stringstream(""), p.info);
    }

    env.end();

    return output.algorithm_end(p.info);
}

#endif

