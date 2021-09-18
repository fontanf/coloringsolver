#include "coloringsolver/algorithms/localsearch_rowweighting.hpp"

#include "coloringsolver/algorithms/greedy.hpp"

#include <thread>

using namespace coloringsolver;

LocalSearchRowWeightingOutput& LocalSearchRowWeightingOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

struct LocalSearchRowWeightingVertex
{
    Counter timestamp = -1;
};

void localsearch_rowweighting_worker(
        const Instance& instance,
        Seed seed,
        LocalSearchRowWeightingOptionalParameters parameters,
        LocalSearchRowWeightingOutput& output,
        Counter thread_id)
{
    std::mt19937_64 generator(seed);

    // Get initial solution.
    Solution solution = (parameters.initial_solution != nullptr)?
        *parameters.initial_solution:
        greedy_dsatur(instance).solution;
    std::stringstream ss;
    ss << "thread " << thread_id << " initial solution";
    output.update_solution(solution, ss, parameters.info);
    parameters.info.output->mutex_solutions.lock();
    parameters.new_solution_callback(output);
    parameters.info.output->mutex_solutions.unlock();

    // Initialize local search structures.
    std::vector<LocalSearchRowWeightingVertex> vertices(instance.number_of_vertices());
    Counter number_of_iterations_without_improvement = 0;
    Counter number_of_improvements = 0;
    std::vector<Penalty> penalties(instance.maximum_degree(), 0);

    for (Counter number_of_iterations = 1; !parameters.info.needs_to_end(); ++number_of_iterations, number_of_iterations_without_improvement++) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && number_of_iterations > parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && number_of_iterations_without_improvement > parameters.maximum_number_of_iterations_without_improvement)
            break;
        if (parameters.maximum_number_of_improvements != -1
                && number_of_improvements > parameters.maximum_number_of_improvements)
            break;
        //if (iterations % 10000 == 0)
        //    std::cout << "it " << iterations << std::endl;

        // If the solution is feasible, we merge two colors.
        // We choose the two merged colors to minimize the penalty of the new
        // solution.
        while (solution.feasible()) {
            // Update best solution
            if (output.solution.number_of_colors() > solution.number_of_colors()) {
                std::stringstream ss;
                ss << "thread " << thread_id
                    << ", it " << number_of_iterations
                    << " (" << number_of_iterations_without_improvement << ")";
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
                assert(c1 != c2);
                penalties[i1][i2] += solution.penalty(e);
            }

            // Find best color combination.
            ColorPos c1_pos_best = -1;
            ColorPos c2_pos_best = -1;
            Penalty p_best = -1;
            for (ColorPos c1_pos = 0; c1_pos < solution.number_of_colors(); ++c1_pos) {
                for (ColorPos c2_pos = c1_pos + 1; c2_pos < solution.number_of_colors(); ++c2_pos) {
                    if (p_best == -1 || p_best > penalties[c1_pos][c2_pos - c1_pos - 1]) {
                        c1_pos_best = c1_pos;
                        c2_pos_best = c2_pos;
                        p_best = penalties[c1_pos][c2_pos - c1_pos - 1];
                    }
                }
            }

            // Apply color merge.
            ColorId c1_best = *(solution.colors_begin() + c1_pos_best);
            ColorId c2_best = *(solution.colors_begin() + c2_pos_best);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v)
                if (solution.color(v) == c2_best)
                    solution.set(v, c1_best);
        }

        // Draw randomly a conflicting edge.
        std::uniform_int_distribution<EdgeId> d_e(0, solution.number_of_conflicts() - 1);
        EdgeId e = *(solution.conflicts().begin() + d_e(generator));

        // Find the best swap move.
        VertexId v_best = -1;
        ColorId  c_best = -1;
        Penalty  p_best = -1;
        for (VertexId v: {instance.edge(e).v1, instance.edge(e).v2}) {
            for (auto it_c = solution.colors_begin(); it_c != solution.colors_end(); ++it_c)
                penalties[*it_c] = 0;
            for (const auto& edge: instance.vertex(v).edges)
                penalties[solution.color(edge.v)] += solution.penalty(edge.e);
            for (auto it_c = solution.colors_begin(); it_c != solution.colors_end(); ++it_c) {
                ColorId c = *it_c;
                if (c == solution.color(v))
                    continue;
                if (p_best == -1 || p_best > penalties[c]) {
                    v_best = v;
                    c_best = c;
                    p_best = penalties[c];
                }
            }
        }
        vertices[v_best].timestamp = number_of_iterations;
        solution.set(v_best, c_best);

        // Update penalties: we increment the penalty of each uncovered element.
        // "reduce" becomes true if we divide by 2 all penalties to avoid
        // integer overflow (this very rarely occur in practice).
        bool reduce = false;
        for (auto it_e = solution.conflicts().begin(); it_e != solution.conflicts().end(); ++it_e) {
            solution.increment_penalty(*it_e);
            if (solution.penalty(*it_e) > std::numeric_limits<Penalty>::max() / instance.number_of_edges())
                reduce = true;
        }
        if (reduce) {
            //std::cout << "reduce" << std::endl;
            for (EdgeId e = 0; e < instance.number_of_edges(); ++e)
                solution.set_penalty(e, (solution.penalty(e) - 1) / 2 + 1);
        }
    }
}

LocalSearchRowWeightingOutput coloringsolver::localsearch_rowweighting(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeightingOptionalParameters parameters)
{
    VER(parameters.info, "*** localsearch_rowweighting ***" << std::endl);

    // Compute initial greedy solution.
    LocalSearchRowWeightingOutput output(instance, parameters.info);

    auto seeds = optimizationtools::bob_floyd(parameters.number_of_threads, std::numeric_limits<Seed>::max(), generator);
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < parameters.number_of_threads; ++thread_id)
        threads.push_back(std::thread(localsearch_rowweighting_worker, std::ref(instance), seeds[thread_id], parameters, std::ref(output), thread_id));

    for (Counter thread_id = 0; thread_id < parameters.number_of_threads; ++thread_id)
        threads[thread_id].join();

    return output.algorithm_end(parameters.info);
}

