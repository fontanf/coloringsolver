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
    info.add_to_json("Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    info.os() << "Number of iterations:  " << number_of_iterations << std::endl;
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
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Row Weighting Local Search" << std::endl
        << std::endl
        << "Parameters" << std::endl
        << "----------" << std::endl
        << "Maximum number of iterations:                      " << parameters.maximum_number_of_iterations << std::endl
        << "Maximum number of iterations without improvement:  " << parameters.maximum_number_of_iterations_without_improvement << std::endl
        << "Maximum number of improvements:                    " << parameters.maximum_number_of_improvements << std::endl
        << "Goal:                                              " << parameters.goal << std::endl
        << std::endl;

    if (instance.adjacency_list_graph() == nullptr) {
        throw std::runtime_error(
                "The 'localsearch_rowweighting' algorithm requires an AdjacencyListGraph.");
    }
    const optimizationtools::AdjacencyListGraph& graph = *instance.adjacency_list_graph();

    LocalSearchRowWeightingOutput output(instance, parameters.info);

    // Get initial solution.
    Solution solution = (parameters.initial_solution != nullptr)?
        *parameters.initial_solution:
        greedy_dsatur(instance).solution;

    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);
    parameters.info.lock();
    parameters.new_solution_callback(output);
    parameters.info.unlock();
    if (output.solution.number_of_colors() <= parameters.goal)
        return output.algorithm_end(parameters.info);
    if (output.solution.number_of_colors() == 1)
        return output.algorithm_end(parameters.info);

    // Initialize local search structures.
    std::vector<LocalSearchRowWeightingVertex> vertices(graph.number_of_vertices());
    Counter number_of_iterations_without_improvement = 0;
    Counter number_of_improvements = 0;
    std::vector<Penalty> penalties(graph.maximum_degree() + 1, 0);
    std::vector<Penalty> solution_penalties(graph.number_of_edges(), 1);
    std::vector<std::pair<VertexId, ColorId>> vcolor_id_bests;
    std::vector<std::pair<ColorId, ColorId>> ccolor_id_bests;

    // Structures for the core.
    std::vector<VertexId> removed_vertices;
    ColorId k = solution.number_of_colors();
    optimizationtools::IndexedSet colors(solution.number_of_colors());
    colors.fill();

    for (output.number_of_iterations = 0; !parameters.info.needs_to_end();
            ++output.number_of_iterations,
            ++number_of_iterations_without_improvement) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations >= parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && number_of_iterations_without_improvement >= parameters.maximum_number_of_iterations_without_improvement)
            break;
        if (parameters.maximum_number_of_improvements != -1
                && number_of_improvements >= parameters.maximum_number_of_improvements)
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
                VertexId vertex_id = *it_v;
                optimizationtools::IndexedSet available_colors = colors;
                auto it = graph.neighbors_begin(vertex_id);
                auto it_end = graph.neighbors_end(vertex_id);
                for (; it != it_end; ++it) {
                    VertexId vertex_id_neighbor = *it;
                    if (solution.contains(vertex_id_neighbor) == 0)
                        continue;
                    ColorId color_id = solution.color(vertex_id_neighbor);
                    available_colors.remove(color_id);
                }
                if (available_colors.empty()) {
                    throw std::runtime_error(
                            "No available color for vertex "
                            + std::to_string(vertex_id)
                            + ".");
                }
                ColorId color_id = *(available_colors.begin());
                solution.set(vertex_id, color_id);
            }
            if (solution.number_of_conflicts() != 0) {
                throw std::runtime_error("Solution has conflicts.");
            }

            // Update best solution
            if (output.solution.number_of_colors() > solution.number_of_colors()) {
                std::stringstream ss;
                ss << "iteration " << output.number_of_iterations;
                output.update_solution(solution, ss, parameters.info);
                parameters.info.lock();
                parameters.new_solution_callback(output);
                parameters.info.unlock();
                number_of_improvements++;
            }

            // Update statistics
            number_of_iterations_without_improvement = 0;

            // Compute positions.
            std::vector<ColorPos> positions(graph.maximum_degree() + 1, -1);
            for (ColorPos color_pos = 0; color_pos < solution.number_of_colors(); ++color_pos) {
                ColorId color_id = *(solution.colors_begin() + color_pos);
                positions[color_id] = color_pos;
            }

            // Initialize penalty structure.
            std::vector<std::vector<Penalty>> penalties(solution.number_of_colors());
            for (ColorPos color_pos = 0;
                    color_pos < solution.number_of_colors();
                    ++color_pos) {
                penalties[color_pos].resize(
                        solution.number_of_colors() - color_pos - 1);
            }

            // Compute penalties.
            for (EdgeId edge_id = 0;
                    edge_id < graph.number_of_edges();
                    ++edge_id) {
                VertexId vertex_id_1 = graph.first_end(edge_id);
                VertexId vertex_id_2 = graph.second_end(edge_id);
                ColorId color_id_1 = solution.color(vertex_id_1);
                ColorId color_id_2 = solution.color(vertex_id_2);
                ColorPos color_pos_1 = positions[color_id_1];
                ColorPos color_pos_2 = positions[color_id_2];
                ColorPos i1 = std::min(color_pos_1, color_pos_2);
                ColorPos i2 = std::max(color_pos_1, color_pos_2) - i1 - 1;
                if (color_id_1 == color_id_2) {
                    throw std::runtime_error(
                            "Vertex " + std::to_string(vertex_id_1)
                            + " and its neighbor vertex "
                            + std::to_string(vertex_id_2)
                            + " have the same color "
                            + std::to_string(color_id_1));
                }
                if (std::numeric_limits<Penalty>::max()
                        - solution_penalties[edge_id]
                        > penalties[i1][i2]) {
                    penalties[i1][i2] += solution_penalties[edge_id];
                } else {
                    penalties[i1][i2] = std::numeric_limits<Penalty>::max();
                }
            }

            // Find best color combination.
            ccolor_id_bests.clear();
            Penalty penalty_best = -1;
            for (ColorPos color_pos_1 = 0; color_pos_1 < solution.number_of_colors(); ++color_pos_1) {
                for (ColorPos color_pos_2 = color_pos_1 + 1; color_pos_2 < solution.number_of_colors(); ++color_pos_2) {
                    if (ccolor_id_bests.empty() || penalty_best > penalties[color_pos_1][color_pos_2 - color_pos_1 - 1]) {
                        ccolor_id_bests.clear();
                        ccolor_id_bests.push_back({color_pos_1, color_pos_2});
                        penalty_best = penalties[color_pos_1][color_pos_2 - color_pos_1 - 1];
                    } else if (!ccolor_id_bests.empty() && penalty_best == penalties[color_pos_1][color_pos_2 - color_pos_1 - 1]) {
                        ccolor_id_bests.push_back({color_pos_1, color_pos_2});
                    }
                }
            }

            // Apply color merge.
            std::uniform_int_distribution<EdgeId> d_cc(0, ccolor_id_bests.size() - 1);
            auto cc = ccolor_id_bests[d_cc(generator)];
            ColorId color_id_1_best = *(solution.colors_begin() + cc.first);
            ColorId color_id_2_best = *(solution.colors_begin() + cc.second);
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id)
                if (solution.color(vertex_id) == color_id_2_best)
                    solution.set(vertex_id, color_id_1_best);
            colors.remove(color_id_2_best);

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
        EdgeId edge_id_cur = *std::next(solution.conflicts().begin(), d_e(generator));

        // Find the best swap move.
        vcolor_id_bests.clear();
        Penalty penalty_best = -1;
        for (VertexId vertex_id: {graph.first_end(edge_id_cur), graph.second_end(edge_id_cur)}) {
            for (ColorId color_id: colors)
                penalties[color_id] = 0;
            for (const auto& edge: graph.edges(vertex_id)) {
                if (solution.contains(edge.v))
                    penalties[solution.color(edge.v)] += solution_penalties[edge.e];
            }
            for (ColorId color_id: colors) {
                if (color_id == solution.color(vertex_id))
                    continue;
                if (vcolor_id_bests.empty()
                        || penalty_best > penalties[color_id]) {
                    vcolor_id_bests.clear();
                    vcolor_id_bests.push_back({vertex_id, color_id});
                    penalty_best = penalties[color_id];
                } else if (!vcolor_id_bests.empty()
                        && penalty_best == penalties[color_id]) {
                    vcolor_id_bests.push_back({vertex_id, color_id});
                }
            }
        }
        std::uniform_int_distribution<EdgeId> d_vc(0, vcolor_id_bests.size() - 1);
        auto vc = vcolor_id_bests[d_vc(generator)];
        // Update vertices structure.
        vertices[vc.first].timestamp = output.number_of_iterations;
        // Update penalties.
        bool reduce = false;

        for (const auto& edge: graph.edges(vc.first)) {
            if (solution.color(edge.v) == vc.second) {
                solution_penalties[edge.e]++;
                if (solution_penalties[edge.e] > std::numeric_limits<Penalty>::max() / 2)
                    reduce = true;
            }
        }
        if (reduce) {
            //std::cout << "reduce" << std::endl;
            for (EdgeId edge_id = 0;
                    edge_id < graph.number_of_edges();
                    ++edge_id) {
                solution_penalties[edge_id] = (solution_penalties[edge_id] - 1) / 2 + 1;
            }
        }
        // Update solution.
        solution.set(vc.first, vc.second);
    }

    return output.algorithm_end(parameters.info);
}

LocalSearchRowWeighting2Output& LocalSearchRowWeighting2Output::algorithm_end(
        optimizationtools::Info& info)
{
    info.add_to_json("Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    info.os() << "Number of iterations:  " << number_of_iterations << std::endl;
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
    parameters.info.os()
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << "Row Weighting Local Search 2" << std::endl
        << std::endl
        << "Parameters" << std::endl
        << "----------" << std::endl
        << "Maximum number of iterations:                      " << parameters.maximum_number_of_iterations << std::endl
        << "Maximum number of iterations without improvement:  " << parameters.maximum_number_of_iterations_without_improvement << std::endl
        << "Maximum number of improvements:                    " << parameters.maximum_number_of_improvements << std::endl
        << "Goal:                                              " << parameters.goal << std::endl
        << std::endl;

    const optimizationtools::AbstractGraph& graph = instance.graph();

    // Compute initial greedy solution.
    LocalSearchRowWeighting2Output output(instance, parameters.info);

    // Get initial solution.
    Solution solution = (parameters.initial_solution != nullptr)?
        *parameters.initial_solution:
        greedy_dsatur(instance).solution;

    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);
    parameters.info.lock();
    parameters.new_solution_callback(output);
    parameters.info.unlock();
    if (output.solution.number_of_colors() <= parameters.goal)
        return output.algorithm_end(parameters.info);
    if (output.solution.number_of_colors() == 1)
        return output.algorithm_end(parameters.info);

    // Initialize local search structures.
    Counter number_of_iterations_without_improvement = 0;
    Counter number_of_improvements = 0;
    std::vector<Penalty> penalties(graph.maximum_degree() + 1, 0);
    std::vector<Penalty> vertex_penalties(graph.number_of_vertices(), 1);
    optimizationtools::IndexedSet uncolored_vertices(graph.number_of_vertices());
    std::vector<std::pair<ColorId, ColorId>> ccolor_id_bests;
    std::vector<ColorId> color_id_bests;

    // Structures for the core.
    std::vector<VertexId> removed_vertices;
    ColorId k = solution.number_of_colors();
    optimizationtools::IndexedSet colors(solution.number_of_colors());
    colors.fill();

    for (output.number_of_iterations = 0; !parameters.info.needs_to_end();
            ++output.number_of_iterations,
            ++number_of_iterations_without_improvement) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations >= parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && number_of_iterations_without_improvement >= parameters.maximum_number_of_iterations_without_improvement)
            break;
        if (parameters.maximum_number_of_improvements != -1
                && number_of_improvements >= parameters.maximum_number_of_improvements)
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
                VertexId vertex_id = *it_v;
                optimizationtools::IndexedSet available_colors = colors;
                auto it = graph.neighbors_begin(vertex_id);
                auto it_end = graph.neighbors_end(vertex_id);
                for (; it != it_end; ++it) {
                    VertexId vertex_id_neighbor = *it;
                    if (solution.contains(vertex_id_neighbor) == 0)
                        continue;
                    ColorId color_id = solution.color(vertex_id_neighbor);
                    available_colors.remove(color_id);
                }
                if (available_colors.empty()) {
                    throw std::runtime_error(
                            "No available color for vertex "
                            + std::to_string(vertex_id)
                            + ".");
                }
                ColorId color_id = *(available_colors.begin());
                solution.set(vertex_id, color_id);
            }
            if (solution.number_of_conflicts() != 0) {
                throw std::runtime_error("Solution has conflicts.");
            }

            // Update best solution
            if (output.solution.number_of_colors() > solution.number_of_colors()) {
                std::stringstream ss;
                ss << "iteration " << output.number_of_iterations;
                output.update_solution(solution, ss, parameters.info);
                parameters.info.lock();
                parameters.new_solution_callback(output);
                parameters.info.unlock();
                number_of_improvements++;
            }

            // Update statistics
            number_of_iterations_without_improvement = 0;

            // Compute positions.
            std::vector<ColorPos> positions(graph.maximum_degree() + 1, -1);
            for (ColorPos color_pos = 0; color_pos < solution.number_of_colors(); ++color_pos) {
                ColorId color_id = *(solution.colors_begin() + color_pos);
                positions[color_id] = color_pos;
            }

            // Initialize penalty structure.
            std::vector<std::vector<Penalty>> penalties(solution.number_of_colors());
            for (ColorPos color_pos = 0;
                    color_pos < solution.number_of_colors();
                    ++color_pos) {
                penalties[color_pos].resize(solution.number_of_colors() - color_pos - 1);
            }

            // Compute penalties.
            for (VertexId vertex_id_1 = 0; vertex_id_1 < graph.number_of_vertices(); ++vertex_id_1) {
                auto it = graph.neighbors_begin(vertex_id_1);
                auto it_end = graph.neighbors_end(vertex_id_1);
                for (; it != it_end; ++it) {
                    VertexId vertex_id_2 = *it;
                    if (vertex_id_2 <= vertex_id_1)
                        continue;
                    ColorId color_id_1 = solution.color(vertex_id_1);
                    ColorId color_id_2 = solution.color(vertex_id_2);
                    ColorPos color_pos_1 = positions[color_id_1];
                    ColorPos color_pos_2 = positions[color_id_2];
                    ColorPos i1 = std::min(color_pos_1, color_pos_2);
                    ColorPos i2 = std::max(color_pos_1, color_pos_2) - i1 - 1;
                    if (color_id_1 == color_id_2) {
                        throw std::runtime_error(
                                "Vertex " + std::to_string(vertex_id_1)
                                + " and its neighbor vertex "
                                + std::to_string(vertex_id_2)
                                + " have the same color "
                                + std::to_string(color_id_1));
                    }
                    if (std::numeric_limits<Penalty>::max()
                            - vertex_penalties[vertex_id_1] - vertex_penalties[vertex_id_2]
                            > penalties[i1][i2]) {
                        penalties[i1][i2] += vertex_penalties[vertex_id_1] + vertex_penalties[vertex_id_2];
                    } else {
                        penalties[i1][i2] = std::numeric_limits<Penalty>::max();
                    }
                }
            }

            // Find best color combination.
            ccolor_id_bests.clear();
            Penalty penalty_best = -1;
            for (ColorPos color_pos_1 = 0;
                    color_pos_1 < solution.number_of_colors();
                    ++color_pos_1) {
                for (ColorPos color_pos_2 = color_pos_1 + 1;
                        color_pos_2 < solution.number_of_colors();
                        ++color_pos_2) {
                    if (ccolor_id_bests.empty() || penalty_best > penalties[color_pos_1][color_pos_2 - color_pos_1 - 1]) {
                        ccolor_id_bests.clear();
                        ccolor_id_bests.push_back({color_pos_1, color_pos_2});
                        penalty_best = penalties[color_pos_1][color_pos_2 - color_pos_1 - 1];
                    } else if (!ccolor_id_bests.empty()
                            && penalty_best == penalties[color_pos_1][color_pos_2 - color_pos_1 - 1]) {
                        ccolor_id_bests.push_back({color_pos_1, color_pos_2});
                    }
                }
            }

            // Apply color merge.
            std::uniform_int_distribution<EdgeId> d_cc(0, ccolor_id_bests.size() - 1);
            auto cc = ccolor_id_bests[d_cc(generator)];
            ColorId color_id_1_best = *(solution.colors_begin() + cc.first);
            ColorId color_id_2_best = *(solution.colors_begin() + cc.second);
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id) {
                if (solution.color(vertex_id) == color_id_2_best)
                    solution.set(vertex_id, color_id_1_best);
            }
            colors.remove(color_id_2_best);

            // Compute core.
            k--;
            if (parameters.enable_core_reduction) {
                removed_vertices = instance.compute_core(k);
                for (VertexId vertex_id: removed_vertices)
                    solution.set(vertex_id, -1);
            }

            // Remove conflicting vertices.
            std::vector<VertexId> vec;
            for (auto p: solution.conflicting_vertices()) {
                VertexId vertex_id = p.first;
                vec.push_back(vertex_id);
            }
            for (VertexId vertex_id: vec) {
                if (solution.contains(vertex_id)) {
                    solution.set(vertex_id, -1);
                    uncolored_vertices.add(vertex_id);
                }
            }

            if (output.solution.number_of_colors() == 2
                    && !solution.feasible())
                return output.algorithm_end(parameters.info);
        }

        // Draw randomly a conflicting edge.
        std::uniform_int_distribution<VertexId> d_v(0, uncolored_vertices.size() - 1);
        VertexId vertex_id_cur = *std::next(uncolored_vertices.begin(), d_v(generator));

        // Find the best swap move.
        color_id_bests.clear();
        Penalty penalty_best = -1;
        for (ColorId color_id: colors)
            penalties[color_id] = 0;
        auto it = graph.neighbors_begin(vertex_id_cur);
        auto it_end = graph.neighbors_end(vertex_id_cur);
        for (; it != it_end; ++it) {
            VertexId vertex_id_neighbor = *it;
            if (solution.contains(vertex_id_neighbor)) {
                penalties[solution.color(vertex_id_neighbor)]
                    += vertex_penalties[vertex_id_neighbor];
            }
        }
        for (ColorId color_id: colors) {
            if (color_id_bests.empty() || penalty_best > penalties[color_id]) {
                color_id_bests.clear();
                color_id_bests.push_back(color_id);
                penalty_best = penalties[color_id];
            } else if (!color_id_bests.empty()
                    && penalty_best == penalties[color_id]) {
                color_id_bests.push_back(color_id);
            }
        }
        std::uniform_int_distribution<EdgeId> d_c(0, color_id_bests.size() - 1);
        ColorId color_id_best = color_id_bests[d_c(generator)];
        // Update penalties.
        bool reduce = false;
        it = graph.neighbors_begin(vertex_id_cur);
        it_end = graph.neighbors_end(vertex_id_cur);
        for (; it != it_end; ++it) {
            VertexId vertex_id_neighbor = *it;
            if (solution.color(vertex_id_neighbor) == color_id_best) {
                vertex_penalties[vertex_id_neighbor]++;
                if (vertex_penalties[vertex_id_neighbor]
                        > std::numeric_limits<Penalty>::max() / 2)
                    reduce = true;
            }
        }
        if (reduce) {
            //std::cout << "reduce" << std::endl;
            for (VertexId vertex_id = 0;
                    vertex_id < graph.number_of_vertices();
                    ++vertex_id) {
                vertex_penalties[vertex_id] = (vertex_penalties[vertex_id] - 1) / 2 + 1;
            }
        }
        // Update solution.
        solution.set(vertex_id_cur, color_id_best);
        uncolored_vertices.remove(vertex_id_cur);
        std::vector<VertexId> vertices_to_remove;
        it = graph.neighbors_begin(vertex_id_cur);
        it_end = graph.neighbors_end(vertex_id_cur);
        for (; it != it_end; ++it) {
            VertexId vertex_id_neighbor = *it;
            if (solution.color(vertex_id_neighbor) == color_id_best) {
                vertices_to_remove.push_back(vertex_id_neighbor);
                uncolored_vertices.add(vertex_id_neighbor);
            }
        }
        for (VertexId vertex_id: vertices_to_remove)
            solution.set(vertex_id, -1);
    }

    return output.algorithm_end(parameters.info);
}

