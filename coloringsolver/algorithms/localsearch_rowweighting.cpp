#include "coloringsolver/algorithms/localsearch_rowweighting.hpp"

#include "coloringsolver/algorithms/greedy.hpp"

#include <thread>

using namespace coloringsolver;

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// localsearch_rowweighting ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

LocalSearchRowWeightingOutput& LocalSearchRowWeightingOutput::algorithm_end(
        optimizationtools::Info& info)
{
    PUT(info, "Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    VER(info, "Number of iterations:  " << number_of_iterations << std::endl);
    return *this;
}

struct LocalSearchRowWeightingVertex
{
    Counter timestamp = -1;
};

LocalSearchRowWeightingOutput coloringsolver::localsearch_rowweighting(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeightingOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    VER(parameters.info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "Row Weighting Local Search" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Maximum number of iterations:                      " << parameters.maximum_number_of_iterations << std::endl
            << "Maximum number of iterations without improvement:  " << parameters.maximum_number_of_iterations_without_improvement << std::endl
            << "Maximum number of improvements:                    " << parameters.maximum_number_of_improvements << std::endl
            << "Goal:                                              " << parameters.goal << std::endl
            << std::endl);

    // Compute initial greedy solution.
    LocalSearchRowWeightingOutput output(instance, parameters.info);

    // Get initial solution.
    Solution solution = (parameters.initial_solution != nullptr)?
        *parameters.initial_solution:
        greedy_dsatur(instance).solution;

    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);
    parameters.info.output->mutex_solutions.lock();
    parameters.new_solution_callback(output);
    parameters.info.output->mutex_solutions.unlock();
    if (output.solution.number_of_colors() <= parameters.goal)
        return output.algorithm_end(parameters.info);

    // Initialize local search structures.
    std::vector<LocalSearchRowWeightingVertex> vertices(instance.number_of_vertices());
    Counter number_of_iterations_without_improvement = 0;
    Counter number_of_improvements = 0;
    std::vector<Penalty> penalties(instance.maximum_degree(), 0);
    std::vector<Penalty> solution_penalties(instance.number_of_edges(), 1);
    std::vector<std::pair<VertexId, ColorId>> vc_bests;
    std::vector<std::pair<ColorId, ColorId>> cc_bests;

    // Structures for the core.
    std::vector<VertexId> removed_vertices;
    ColorId k = solution.number_of_colors();
    optimizationtools::IndexedSet colors(solution.number_of_colors());
    colors.fill();

    for (output.number_of_iterations = 1; !parameters.info.needs_to_end(); ++output.number_of_iterations, number_of_iterations_without_improvement++) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations > parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && number_of_iterations_without_improvement > parameters.maximum_number_of_iterations_without_improvement)
            break;
        if (parameters.maximum_number_of_improvements != -1
                && number_of_improvements > parameters.maximum_number_of_improvements)
            break;
        if (output.solution.number_of_colors() <= parameters.goal)
            break;
        //if (iterations % 10000 == 0)
        //    std::cout << "it " << iterations << std::endl;

        // If the solution is feasible, we merge two colors.
        // We choose the two merged colors to minimize the penalty of the new
        // solution.
        while (solution.number_of_conflicts() == 0) {
            // Give a color to vertices outside of the core.
            for (auto it_v = removed_vertices.rbegin();
                    it_v != removed_vertices.rend(); ++it_v) {
                VertexId v = *it_v;
                optimizationtools::IndexedSet available_colors = colors;
                for (const auto& edge: instance.vertex(v).edges) {
                    if (solution.contains(edge.v) == 0)
                        continue;
                    ColorId c = solution.color(edge.v);
                    available_colors.remove(c);
                }
                if (available_colors.empty()) {
                    throw std::runtime_error(
                            "No available color for vertex "
                            + std::to_string(v)
                            + ".");
                }
                ColorId c = *(available_colors.begin());
                solution.set(v, c);
            }
            if (solution.number_of_conflicts() != 0) {
                throw std::runtime_error("Solution has conflicts.");
            }

            // Update best solution
            if (output.solution.number_of_colors() > solution.number_of_colors()) {
                std::stringstream ss;
                ss << "iteration " << output.number_of_iterations;
                output.update_solution(solution, ss, parameters.info);
                parameters.info.output->mutex_solutions.lock();
                parameters.new_solution_callback(output);
                parameters.info.output->mutex_solutions.unlock();
                number_of_improvements++;
            }

            // Update statistics
            number_of_iterations_without_improvement = 0;

            // Compute positions.
            std::vector<ColorPos> positions(instance.maximum_degree(), -1);
            for (ColorPos c_pos = 0; c_pos < solution.number_of_colors(); ++c_pos) {
                ColorId c = *(solution.colors_begin() + c_pos);
                positions[c] = c_pos;
            }

            // Initialize penalty structure.
            std::vector<std::vector<Penalty>> penalties(solution.number_of_colors());
            for (ColorPos c_pos = 0; c_pos < solution.number_of_colors(); ++c_pos)
                penalties[c_pos].resize(solution.number_of_colors() - c_pos - 1);

            // Compute penalties.
            for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
                VertexId v1 = instance.edge(e).v1;
                VertexId v2 = instance.edge(e).v2;
                ColorId c1 = solution.color(v1);
                ColorId c2 = solution.color(v2);
                ColorPos c1_pos = positions[c1];
                ColorPos c2_pos = positions[c2];
                ColorPos i1 = std::min(c1_pos, c2_pos);
                ColorPos i2 = std::max(c1_pos, c2_pos) - i1 - 1;
                if (c1 == c2) {
                    throw std::runtime_error(
                            "Vertex " + std::to_string(v1)
                            + " and its neighbor vertex "
                            + std::to_string(v2)
                            + " have the same color "
                            + std::to_string(c1));
                }
                if (std::numeric_limits<Penalty>::max() - solution_penalties[e]
                        > penalties[i1][i2]) {
                    penalties[i1][i2] += solution_penalties[e];
                } else {
                    penalties[i1][i2] = std::numeric_limits<Penalty>::max();
                }
            }

            // Find best color combination.
            cc_bests.clear();
            Penalty p_best = -1;
            for (ColorPos c1_pos = 0; c1_pos < solution.number_of_colors(); ++c1_pos) {
                for (ColorPos c2_pos = c1_pos + 1; c2_pos < solution.number_of_colors(); ++c2_pos) {
                    if (cc_bests.empty() || p_best > penalties[c1_pos][c2_pos - c1_pos - 1]) {
                        cc_bests.clear();
                        cc_bests.push_back({c1_pos, c2_pos});
                        p_best = penalties[c1_pos][c2_pos - c1_pos - 1];
                    } else if (!cc_bests.empty() && p_best == penalties[c1_pos][c2_pos - c1_pos - 1]) {
                        cc_bests.push_back({c1_pos, c2_pos});
                    }
                }
            }

            // Apply color merge.
            std::uniform_int_distribution<EdgeId> d_cc(0, cc_bests.size() - 1);
            auto cc = cc_bests[d_cc(generator)];
            ColorId c1_best = *(solution.colors_begin() + cc.first);
            ColorId c2_best = *(solution.colors_begin() + cc.second);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                if (solution.color(v) == c2_best)
                    solution.set(v, c1_best);
            colors.remove(c2_best);

            // Compute core.
            k--;
            if (parameters.enable_core_reduction) {
                removed_vertices = instance.compute_core(k);
                for (VertexId v: removed_vertices)
                    solution.set(v, -1);
            }

            if (output.solution.number_of_colors() == 2
                    && !solution.feasible())
                return output.algorithm_end(parameters.info);
        }

        // Draw randomly a conflicting edge.
        std::uniform_int_distribution<EdgeId> d_e(0, solution.number_of_conflicts() - 1);
        EdgeId e_cur = *std::next(solution.conflicts().begin(), d_e(generator));

        // Find the best swap move.
        vc_bests.clear();
        Penalty p_best = -1;
        for (VertexId v: {instance.edge(e_cur).v1, instance.edge(e_cur).v2}) {
            for (ColorId c: colors)
                penalties[c] = 0;
            for (const auto& edge: instance.vertex(v).edges)
                if (solution.contains(edge.v))
                    penalties[solution.color(edge.v)] += solution_penalties[edge.e];
            for (ColorId c: colors) {
                if (c == solution.color(v))
                    continue;
                if (vc_bests.empty() || p_best > penalties[c]) {
                    vc_bests.clear();
                    vc_bests.push_back({v, c});
                    p_best = penalties[c];
                } else if (!vc_bests.empty() && p_best == penalties[c]) {
                    vc_bests.push_back({v, c});
                }
            }
        }
        std::uniform_int_distribution<EdgeId> d_vc(0, vc_bests.size() - 1);
        auto vc = vc_bests[d_vc(generator)];
        // Update vertices structure.
        vertices[vc.first].timestamp = output.number_of_iterations;
        // Update penalties.
        bool reduce = false;
        for (const auto& edge: instance.vertex(vc.first).edges) {
            if (solution.color(edge.v) == vc.second) {
                solution_penalties[edge.e]++;
                if (solution_penalties[edge.e] > std::numeric_limits<Penalty>::max() / 2)
                    reduce = true;
            }
        }
        if (reduce) {
            //std::cout << "reduce" << std::endl;
            for (EdgeId e = 0; e < instance.number_of_edges(); ++e)
                solution_penalties[e] = (solution_penalties[e] - 1) / 2 + 1;
        }
        // Update solution.
        solution.set(vc.first, vc.second);
    }

    return output.algorithm_end(parameters.info);
}

LocalSearchRowWeighting2Output& LocalSearchRowWeighting2Output::algorithm_end(
        optimizationtools::Info& info)
{
    PUT(info, "Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    VER(info, "Number of iterations:  " << number_of_iterations << std::endl);
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////// localsearch_rowweighting_2 //////////////////////////
////////////////////////////////////////////////////////////////////////////////

LocalSearchRowWeighting2Output coloringsolver::localsearch_rowweighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters)
{
    init_display(instance, parameters.info);
    VER(parameters.info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "Row Weighting Local Search 2" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Maximum number of iterations:                      " << parameters.maximum_number_of_iterations << std::endl
            << "Maximum number of iterations without improvement:  " << parameters.maximum_number_of_iterations_without_improvement << std::endl
            << "Maximum number of improvements:                    " << parameters.maximum_number_of_improvements << std::endl
            << "Goal:                                              " << parameters.goal << std::endl
            << std::endl);

    // Compute initial greedy solution.
    LocalSearchRowWeighting2Output output(instance, parameters.info);

    // Get initial solution.
    Solution solution = (parameters.initial_solution != nullptr)?
        *parameters.initial_solution:
        greedy_dsatur(instance).solution;

    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);
    parameters.info.output->mutex_solutions.lock();
    parameters.new_solution_callback(output);
    parameters.info.output->mutex_solutions.unlock();
    if (output.solution.number_of_colors() <= parameters.goal)
        return output.algorithm_end(parameters.info);

    // Initialize local search structures.
    Counter number_of_iterations_without_improvement = 0;
    Counter number_of_improvements = 0;
    std::vector<Penalty> penalties(instance.maximum_degree(), 0);
    std::vector<Penalty> vertex_penalties(instance.number_of_vertices(), 1);
    optimizationtools::IndexedSet uncolored_vertices(instance.number_of_vertices());
    std::vector<std::pair<ColorId, ColorId>> cc_bests;
    std::vector<ColorId> c_bests;

    // Structures for the core.
    std::vector<VertexId> removed_vertices;
    ColorId k = solution.number_of_colors();
    optimizationtools::IndexedSet colors(solution.number_of_colors());
    colors.fill();

    for (output.number_of_iterations = 1; !parameters.info.needs_to_end(); ++output.number_of_iterations, number_of_iterations_without_improvement++) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations > parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && number_of_iterations_without_improvement > parameters.maximum_number_of_iterations_without_improvement)
            break;
        if (parameters.maximum_number_of_improvements != -1
                && number_of_improvements > parameters.maximum_number_of_improvements)
            break;
        if (output.solution.number_of_colors() <= parameters.goal)
            break;
        //if (iterations % 10000 == 0)
        //    std::cout << "it " << iterations << std::endl;

        // If the solution is feasible, we merge two colors.
        // We choose the two merged colors to minimize the penalty of the new
        // solution.
        while (uncolored_vertices.empty()) {
            // Give a color to vertices outside of the core.
            for (auto it_v = removed_vertices.rbegin();
                    it_v != removed_vertices.rend(); ++it_v) {
                VertexId v = *it_v;
                optimizationtools::IndexedSet available_colors = colors;
                for (const auto& edge: instance.vertex(v).edges) {
                    if (solution.contains(edge.v) == 0)
                        continue;
                    ColorId c = solution.color(edge.v);
                    available_colors.remove(c);
                }
                if (available_colors.empty()) {
                    throw std::runtime_error(
                            "No available color for vertex "
                            + std::to_string(v)
                            + ".");
                }
                ColorId c = *(available_colors.begin());
                solution.set(v, c);
            }
            if (solution.number_of_conflicts() != 0) {
                throw std::runtime_error("Solution has conflicts.");
            }

            // Update best solution
            if (output.solution.number_of_colors() > solution.number_of_colors()) {
                std::stringstream ss;
                ss << "iteration " << output.number_of_iterations;
                output.update_solution(solution, ss, parameters.info);
                parameters.info.output->mutex_solutions.lock();
                parameters.new_solution_callback(output);
                parameters.info.output->mutex_solutions.unlock();
                number_of_improvements++;
            }

            // Update statistics
            number_of_iterations_without_improvement = 0;

            // Compute positions.
            std::vector<ColorPos> positions(instance.maximum_degree(), -1);
            for (ColorPos c_pos = 0; c_pos < solution.number_of_colors(); ++c_pos) {
                ColorId c = *(solution.colors_begin() + c_pos);
                positions[c] = c_pos;
            }

            // Initialize penalty structure.
            std::vector<std::vector<Penalty>> penalties(solution.number_of_colors());
            for (ColorPos c_pos = 0; c_pos < solution.number_of_colors(); ++c_pos)
                penalties[c_pos].resize(solution.number_of_colors() - c_pos - 1);

            // Compute penalties.
            for (EdgeId e = 0; e < instance.number_of_edges(); ++e) {
                VertexId v1 = instance.edge(e).v1;
                VertexId v2 = instance.edge(e).v2;
                ColorId c1 = solution.color(v1);
                ColorId c2 = solution.color(v2);
                ColorPos c1_pos = positions[c1];
                ColorPos c2_pos = positions[c2];
                ColorPos i1 = std::min(c1_pos, c2_pos);
                ColorPos i2 = std::max(c1_pos, c2_pos) - i1 - 1;
                if (c1 == c2) {
                    throw std::runtime_error(
                            "Vertex " + std::to_string(v1)
                            + " and its neighbor vertex "
                            + std::to_string(v2)
                            + " have the same color "
                            + std::to_string(c1));
                }
                if (std::numeric_limits<Penalty>::max()
                        - vertex_penalties[v1] - vertex_penalties[v2]
                        > penalties[i1][i2]) {
                    penalties[i1][i2] += vertex_penalties[v1] + vertex_penalties[v2];
                } else {
                    penalties[i1][i2] = std::numeric_limits<Penalty>::max();
                }
            }

            // Find best color combination.
            cc_bests.clear();
            Penalty p_best = -1;
            for (ColorPos c1_pos = 0; c1_pos < solution.number_of_colors(); ++c1_pos) {
                for (ColorPos c2_pos = c1_pos + 1; c2_pos < solution.number_of_colors(); ++c2_pos) {
                    if (cc_bests.empty() || p_best > penalties[c1_pos][c2_pos - c1_pos - 1]) {
                        cc_bests.clear();
                        cc_bests.push_back({c1_pos, c2_pos});
                        p_best = penalties[c1_pos][c2_pos - c1_pos - 1];
                    } else if (!cc_bests.empty() && p_best == penalties[c1_pos][c2_pos - c1_pos - 1]) {
                        cc_bests.push_back({c1_pos, c2_pos});
                    }
                }
            }

            // Apply color merge.
            std::uniform_int_distribution<EdgeId> d_cc(0, cc_bests.size() - 1);
            auto cc = cc_bests[d_cc(generator)];
            ColorId c1_best = *(solution.colors_begin() + cc.first);
            ColorId c2_best = *(solution.colors_begin() + cc.second);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                if (solution.color(v) == c2_best)
                    solution.set(v, c1_best);
            colors.remove(c2_best);

            // Compute core.
            k--;
            if (parameters.enable_core_reduction) {
                removed_vertices = instance.compute_core(k);
                for (VertexId v: removed_vertices)
                    solution.set(v, -1);
            }

            // Remove conflicting vertices.
            std::vector<VertexId> vec;
            for (EdgeId e: solution.conflicts()) {
                VertexId v1 = instance.edge(e).v1;
                VertexId v2 = instance.edge(e).v2;
                vec.push_back(v1);
                vec.push_back(v2);
            }
            for (VertexId v: vec) {
                if (solution.contains(v)) {
                    solution.set(v, -1);
                    uncolored_vertices.add(v);
                }
            }

            if (output.solution.number_of_colors() == 2
                    && !solution.feasible())
                return output.algorithm_end(parameters.info);
        }

        // Draw randomly a conflicting edge.
        std::uniform_int_distribution<VertexId> d_v(0, uncolored_vertices.size() - 1);
        VertexId v_cur = *std::next(uncolored_vertices.begin(), d_v(generator));

        // Find the best swap move.
        c_bests.clear();
        Penalty  p_best = -1;
        for (ColorId c: colors)
            penalties[c] = 0;
        for (const auto& edge: instance.vertex(v_cur).edges)
            if (solution.contains(edge.v))
                penalties[solution.color(edge.v)] += vertex_penalties[edge.v];
        for (ColorId c: colors) {
            if (c_bests.empty() || p_best > penalties[c]) {
                c_bests.clear();
                c_bests.push_back(c);
                p_best = penalties[c];
            } else if (!c_bests.empty() && p_best == penalties[c]) {
                c_bests.push_back(c);
            }
        }
        std::uniform_int_distribution<EdgeId> d_c(0, c_bests.size() - 1);
        ColorId c_best = c_bests[d_c(generator)];
        // Update penalties.
        bool reduce = false;
        for (const auto& edge: instance.vertex(v_cur).edges) {
            if (solution.color(edge.v) == c_best) {
                vertex_penalties[edge.v]++;
                if (vertex_penalties[edge.v] > std::numeric_limits<Penalty>::max() / 2)
                    reduce = true;
            }
        }
        if (reduce) {
            //std::cout << "reduce" << std::endl;
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                vertex_penalties[v] = (vertex_penalties[v] - 1) / 2 + 1;
        }
        // Update solution.
        solution.set(v_cur, c_best);
        uncolored_vertices.remove(v_cur);
        for (const auto& edge: instance.vertex(v_cur).edges) {
            if (solution.color(edge.v) == c_best) {
                solution.set(edge.v, -1);
                uncolored_vertices.add(edge.v);
            }
        }
    }

    return output.algorithm_end(parameters.info);
}

