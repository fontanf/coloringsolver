#include "coloringsolver/algorithms/localsearch.hpp"

#include "coloringsolver/algorithms/greedy.hpp"

#include "localsearchsolver/a_star_local_search.hpp"

using namespace coloringsolver;
using namespace localsearchsolver;

namespace coloringsolver
{

class LocalScheme
{

public:

    /** Global cost: <Number of colors, Number of conflicts> */
    using GlobalCost = std::tuple<ColorId, EdgeId>;

    inline ColorId&         number_of_colors(GlobalCost& global_cost) { return std::get<0>(global_cost); }
    inline EdgeId&       number_of_conflicts(GlobalCost& global_cost) { return std::get<1>(global_cost); }
    inline ColorId    number_of_colors(const GlobalCost& global_cost) { return std::get<0>(global_cost); }
    inline EdgeId  number_of_conflicts(const GlobalCost& global_cost) { return std::get<1>(global_cost); }

    static GlobalCost global_cost_worst()
    {
        return {
            std::numeric_limits<ColorId>::max(),
            std::numeric_limits<EdgeId>::max(),
        };
    }

    /*
     * Solutions.
     */

    using CompactSolution = std::vector<ColorId>;

    struct CompactSolutionHasher
    {
        std::hash<ColorId> hasher;

        inline bool operator()(
                const std::shared_ptr<CompactSolution>& compact_solution_1,
                const std::shared_ptr<CompactSolution>& compact_solution_2) const
        {
            return *compact_solution_1 == *compact_solution_2;
        }

        inline std::size_t operator()(
                const std::shared_ptr<CompactSolution>& compact_solution) const
        {
            size_t hash = 0;
            for (ColorId c: *compact_solution)
                optimizationtools::hash_combine(hash, hasher(c));
            return hash;
        }
    };

    inline CompactSolutionHasher compact_solution_hasher() const { return CompactSolutionHasher(); }

    struct Solution
    {
        Solution(const Instance& instance):
            map(instance.number_of_vertices(), instance.maximum_degree()),
            conflicting_vertices(instance.number_of_vertices(), 0) {  }

        optimizationtools::DoublyIndexedMap map;
        optimizationtools::IndexedMap<EdgeId> conflicting_vertices;
        EdgeId total_number_of_conflicts = 0;
    };

    CompactSolution solution2compact(const Solution& solution)
    {
        std::vector<ColorId> compact_solution(instance_.number_of_vertices(), -1);
        for (VertexId v = 0; v < instance_.number_of_vertices(); ++v)
            compact_solution[v] = solution.map[v];
        return compact_solution;
    }

    Solution compact2solution(const CompactSolution& compact_solution)
    {
        auto solution = empty_solution();
        for (VertexId v = 0; v < instance_.number_of_vertices(); ++v) {
            ColorId c = compact_solution[v];
            set(solution, v, c);
        }
        return solution;
    }

    /*
     * Constructors and destructor.
     */

    struct Parameters
    {
    };

    LocalScheme(
            const Instance& instance,
            Parameters parameters):
        instance_(instance),
        parameters_(parameters)
    {
    }

    LocalScheme(const LocalScheme& local_scheme):
        LocalScheme(local_scheme.instance_, local_scheme.parameters_) { }

    virtual ~LocalScheme() { }

    /*
     * Initial solutions.
     */

    inline Solution empty_solution() const
    {
        return Solution(instance_);
    }

    inline Solution initial_solution(
            Counter,
            std::mt19937_64& generator)
    {
        return solution(greedy_dsatur(instance_).solution, generator);
    }

    inline Solution solution(
            const coloringsolver::Solution& solution,
            std::mt19937_64&) const
    {
        Solution solution_new = empty_solution();

        for (VertexId v = 0; v < instance_.number_of_vertices(); ++v) {
            if (!solution.contains(v))
                continue;
            ColorId c = solution.color(v);
            set(solution_new, v, c);
        }

        return solution_new;
    }

    /*
     * Solution properties.
     */

    inline GlobalCost global_cost(const Solution& solution) const
    {
        return {
            solution.map.number_of_values(),
            solution.total_number_of_conflicts,
        };
    }

    /*
     * Local search.
     */

    struct Move
    {
        // If the solution is not feasible, change color of conflicting vertex
        // v to c.
        VertexId v;
        ColorId c;

        // If the solution is feasible, merge colors c1 and c2.
        ColorId c1;
        ColorId c2;

        GlobalCost global_cost;
    };

    static Move move_null() { return {-1, -1, -1, -1, global_cost_worst()}; }

    struct MoveHasher
    {
        std::hash<EdgeId> hasher_1;
        std::hash<ColorId> hasher_2;

        inline bool hashable(const Move&) const { return true; }

        inline bool operator()(
                const Move& move_1,
                const Move& move_2) const
        {
            return move_1.v == move_2.v
                && move_1.c == move_2.c
                && move_1.c1 == move_2.c1
                && move_1.c2 == move_2.c2;
        }

        inline std::size_t operator()(
                const Move& move) const
        {
            size_t hash = hasher_1(move.v);
            optimizationtools::hash_combine(hash, hasher_2(move.c));
            optimizationtools::hash_combine(hash, hasher_2(move.c1));
            optimizationtools::hash_combine(hash, hasher_2(move.c2));
            return hash;
        }
    };

    inline MoveHasher move_hasher() const { return MoveHasher(); }

    inline std::vector<Move> perturbations(
            const Solution& solution,
            std::mt19937_64&)
    {
        //std::cout << "perturbations start" << std::endl;
        std::vector<Move> moves;
        if (solution.total_number_of_conflicts > 0) {
            for (auto p: solution.conflicting_vertices) {
                VertexId v = p.first;
                for (auto it_c = solution.map.values_begin();
                        it_c != solution.map.values_end(); ++it_c) {
                    ColorId c = *it_c;
                    if (c == solution.map[v])
                        continue;
                    Move move;
                    move.v = v;
                    move.c = c;
                    move.c1 = -1;
                    move.c2 = -1;
                    move.global_cost = cost_set(solution, v, c);
                    moves.push_back(move);
                }
            }
        } else {
            // Compute positions.
            std::vector<ColorPos> positions(instance_.maximum_degree(), -1);
            for (ColorPos c_pos = 0; c_pos < solution.map.number_of_values(); ++c_pos) {
                ColorId c = *(solution.map.values_begin() + c_pos);
                positions[c] = c_pos;
            }

            // Initialize penalty structure.
            std::vector<std::vector<EdgeId>> number_of_conflicts(solution.map.number_of_values());
            for (ColorPos c_pos = 0; c_pos < solution.map.number_of_values(); ++c_pos)
                number_of_conflicts[c_pos].resize(solution.map.number_of_values() - c_pos - 1, 0);

            // Compute penalties.
            for (EdgeId e = 0; e < instance_.number_of_edges(); ++e) {
                VertexId v1 = instance_.edge(e).v1;
                VertexId v2 = instance_.edge(e).v2;
                ColorId c1 = solution.map[v1];
                ColorId c2 = solution.map[v2];
                ColorPos c1_pos = positions[c1];
                ColorPos c2_pos = positions[c2];
                ColorPos i1 = std::min(c1_pos, c2_pos);
                ColorPos i2 = std::max(c1_pos, c2_pos) - i1 - 1;
                assert(c1 != c2);
                number_of_conflicts[i1][i2]++;
            }

            for (ColorPos c1_pos = 0; c1_pos < solution.map.number_of_values(); ++c1_pos) {
                ColorId c1 = *(solution.map.values_begin() + c1_pos);
                for (ColorPos c2_pos = c1_pos + 1; c2_pos < solution.map.number_of_values(); ++c2_pos) {
                    ColorId c2 = *(solution.map.values_begin() + c2_pos);
                    Move move;
                    move.v = -1;
                    move.c = -1;
                    move.c1 = c1;
                    move.c2 = c2;
                    move.global_cost = {
                        solution.map.number_of_values() - 1,
                        number_of_conflicts[c1_pos][c2_pos - c1_pos - 1]};
                    moves.push_back(move);
                }
            }
        }
        //std::cout << "perturbations end" << std::endl;
        return moves;
    }

    inline void apply_move(Solution& solution, const Move& move)
    {
        //std::cout << "apply_move start" << std::endl;
        if (solution.total_number_of_conflicts > 0) {
            set(solution, move.v, move.c);
        } else {
            while (solution.map.number_of_elements(move.c2) > 0) {
                VertexId v = *solution.map.begin(move.c2);
                set(solution, v, move.c1);
            }
        }
        //std::cout << "apply_move end" << std::endl;
    }

    inline void local_search(
            Solution& solution,
            std::mt19937_64& generator,
            const Move& tabu = move_null())
    {
        //std::cout << "local_search start" << std::endl;
        Counter it = 0;
        for (;; ++it) {
            //GlobalCost gc_cur = global_cost(solution);
            //std::cout << "it " << it
            //    << " c " << to_string(gc_cur)
            //    << std::endl;
            //print(std::cout, solution);

            VertexId v_best = -1;
            ColorId c_best = -1;
            GlobalCost gc_best = global_cost(solution);

            vertices_.clear();
            for (auto p: solution.conflicting_vertices)
                vertices_.push_back(p.first);
            std::shuffle(vertices_.begin(), vertices_.end(), generator);

            colors_.clear();
            for (auto it_c = solution.map.values_begin();
                    it_c != solution.map.values_end(); ++it_c)
                colors_.push_back(*it_c);
            std::shuffle(colors_.begin(), colors_.end(), generator);

            for (VertexId v: vertices_) {
                if (v == tabu.v)
                    continue;
                ColorId c_prev = solution.map[v];
                for (ColorId c: colors_) {
                    if (c == c_prev)
                        continue;
                    GlobalCost gc = cost_set(solution, v, c);
                    if (gc >= gc_best)
                        continue;
                    if (v_best != -1 && !dominates(gc, gc_best))
                        continue;
                    v_best = v;
                    c_best = c;
                    gc_best = gc;
                }
            }
            if (v_best == -1)
                break;
            set(solution, v_best, c_best);
            if (global_cost(solution) != gc_best) {
                throw std::logic_error("Costs do not match:\n"
                        "* Expected new cost: " + to_string(gc_best) + "\n"
                        + "* Actual new cost: " + to_string(global_cost(solution)) + "\n");
            }
        }
        //print(std::cout, solution);
        //std::cout << "local_search end" << std::endl;
    }

    /*
     * Outputs.
     */

    std::ostream& print(
            std::ostream &os,
            const Solution& solution)
    {
        os << "Number of colors: " << solution.map.number_of_values() << std::endl;
        os << "Number of conflicts: " << solution.total_number_of_conflicts << std::endl;
        return os;
    }

    inline void write(const Solution&, std::string) const { return; }

private:

    /*
     * Manipulate solutions.
     */

    inline void set(
            Solution& solution,
            VertexId v,
            ColorId c) const
    {
        // Update conflicts_.
        for (const auto& edge: instance_.vertex(v).edges) {
            if (!solution.map.contains(edge.v))
                continue;
            if (solution.map.contains(v)
                    && solution.map[edge.v] == solution.map[v]) {
                solution.conflicting_vertices.set(
                        edge.v, solution.conflicting_vertices[edge.v] - 1);
                solution.conflicting_vertices.set(
                        v, solution.conflicting_vertices[v] - 1);
                solution.total_number_of_conflicts--;
            }
            if (solution.map[edge.v] == c) {
                solution.conflicting_vertices.set(
                        edge.v, solution.conflicting_vertices[edge.v] + 1);
                solution.conflicting_vertices.set(
                        v, solution.conflicting_vertices[v] + 1);
                solution.total_number_of_conflicts++;
            }
        }
        solution.map.set(v, c);
    }

    inline GlobalCost cost_set(
            const Solution& solution,
            VertexId v,
            ColorId c)
    {
        // Update conflicts_.
        GlobalCost gc = global_cost(solution);
        for (const auto& edge: instance_.vertex(v).edges) {
            if (!solution.map.contains(edge.v))
                continue;
            if (solution.map[edge.v] == solution.map[v])
                number_of_conflicts(gc)--;
            if (solution.map[edge.v] == c)
                number_of_conflicts(gc)++;
        }
        return gc;
    }

    /*
     * Private attributes.
     */

    const Instance& instance_;
    Parameters parameters_;

    std::vector<VertexId> vertices_;
    std::vector<ColorId> colors_;

};

}

LocalSearchOutput& LocalSearchOutput::algorithm_end(
        optimizationtools::Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

LocalSearchOutput coloringsolver::localsearch(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchOptionalParameters parameters)
{
    VER(parameters.info, "*** localsearch ***" << std::endl);

    LocalSearchOutput output(instance, parameters.info);

    // Create LocalScheme.
    LocalScheme::Parameters parameters_local_scheme;
    LocalScheme local_scheme(instance, parameters_local_scheme);

    // Run A*.
    AStarLocalSearchOptionalParameters<LocalScheme> parameters_a_star;
    //parameters_a_star.info.set_verbose(true);
    parameters_a_star.info.set_time_limit(parameters.info.remaining_time());
    parameters_a_star.maximum_number_of_nodes = parameters.maximum_number_of_nodes;
    parameters_a_star.number_of_threads_1 = 1;
    parameters_a_star.number_of_threads_2 = parameters.number_of_threads;
    parameters_a_star.initial_solution_ids = std::vector<Counter>(
            parameters_a_star.number_of_threads_2, 0);
    if (parameters.initial_solution == nullptr
            || !parameters.initial_solution->feasible()) {
        parameters_a_star.initial_solution_ids = std::vector<Counter>(
                parameters_a_star.number_of_threads_2, 0);
    } else {
        LocalScheme::Solution solution = local_scheme.solution(*parameters.initial_solution, generator);
        parameters_a_star.initial_solution_ids = {};
        parameters_a_star.initial_solutions = {solution};
    }
    parameters_a_star.new_solution_callback
        = [&instance, &parameters, &output](
                const LocalScheme::Solution& solution)
        {
            if (solution.total_number_of_conflicts > 0)
                return;
            Solution sol(instance);
            for (VertexId v = 0; v < instance.number_of_vertices(); ++v) {
                ColorId c = solution.map[v];
                sol.set(v, c);
            }
            std::stringstream ss;
            output.update_solution(sol, ss, parameters.info);
        };
    a_star_local_search(local_scheme, parameters_a_star);

    return output.algorithm_end(parameters.info);
}

