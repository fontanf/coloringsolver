#pragma once

#include "coloringsolver/instance.hpp"

#include "optimizationtools/containers/indexed_map.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"

#include <unordered_set>

namespace coloringsolver
{

/**
 * Solution class for a 'coloringsolver' problem.
 */
class Solution
{

public:

    /*
     * Constructors and destructor
     */

    /** Create an empty solution. */
    Solution(const Instance& instance);

    /** Create a solution from a certificate file. */
    Solution(
            const Instance& instance,
            std::string certificate_path);

    /*
     * Getters
     */

    /** Get the instance. */
    const Instance& instance() const { return *instance_; }

    /** Return 'true' iff the solution is feasible. */
    bool feasible() const { return number_of_vertices() == instance().graph().number_of_vertices() && number_of_conflicts() == 0; };

    /** Get the number of colors used in the solution. */
    ColorId number_of_colors() const { return map_.number_of_values(); }

    /** Return 'true' iff a color is assigned to vertex v in the solution. */
    bool contains(VertexId vertex_id) const { return map_.contains(vertex_id); }

    /** Get the color of vertex v. */
    ColorId color(VertexId vertex_id) const { return map_[vertex_id]; }

    /** Get the number of vertices with an assigned color. */
    VertexPos number_of_vertices() const { return map_.number_of_elements(); }

    /** Get the number of vertices with color c. */
    VertexPos number_of_vertices(ColorId color_id) const { return map_.number_of_elements(color_id); }

    /** Get the number of conflitcs in the solution. */
    EdgeId number_of_conflicts() const { return total_number_of_conflicts_; }

    /** Get a begin iterator to the colors. */
    std::vector<ColorId>::const_iterator colors_begin() const { return map_.values_begin(); }

    /** Get an end iterator to the colors. */
    std::vector<ColorId>::const_iterator colors_end() const { return map_.values_end(); }

    /** Get the set of conflicting edges. */
    const std::unordered_set<EdgeId>& conflicts() const { return conflicts_; }

    /** Get the set of conflicting vertices. */
    const optimizationtools::IndexedMap<VertexPos>& conflicting_vertices() const { return number_of_conflicts_; }

    /*
     * Setters
     */

    /**
     * Set color c to vertex v.
     *
     * If 'check' is 'false', infeasibilities are not updated. This makes the
     * method faster, but should only be used if this change of color doesn't
     * remove or add any conflict.
     */
    inline void set(
            VertexId vertex_id,
            ColorId color_id,
            bool check = true);

    /*
     * Export
     */

    /** Write the solution to a file. */
    void write(std::string certificate_path) const;

private:

    /*
     * Private attributes
     */

    /** Instance. */
    const Instance* instance_;

    /** Map storing the color assigned to each vertex. */
    optimizationtools::DoublyIndexedMap map_;

    /** Set of conflicting edges. */
    std::unordered_set<EdgeId> conflicts_;

    /** Conflicting vertices. */
    optimizationtools::IndexedMap<VertexPos> number_of_conflicts_;

    /** Number of conflicts. */
    EdgeId total_number_of_conflicts_ = 0;

};

/** Stream insertion operator. */
std::ostream& operator<<(std::ostream& os, const Solution& solution);

void Solution::set(
        VertexId vertex_id,
        ColorId color_id,
        bool check)
{
    const optimizationtools::AbstractGraph& graph = instance().graph();

    // Checks.
    graph.check_vertex_index(vertex_id);
    if (color_id < -1 || color_id >= graph.number_of_vertices()) {
        throw std::out_of_range(
                "Invalid color value: \"" + std::to_string(color_id) + "\"."
                + " Color values should belong to [-1, "
                + std::to_string(number_of_vertices() - 1) + "].");
    }

    // Update conflicts_.
    if (check) {
        if (instance().adjacency_list_graph() != nullptr) {
            for (const auto& edge: instance().adjacency_list_graph()->edges(vertex_id)) {
                // Remove old conflicts.
                if (contains(edge.v) && color(edge.v) == color(vertex_id)) {
                    total_number_of_conflicts_--;
                    conflicts_.erase(edge.e);
                    number_of_conflicts_.set(vertex_id, number_of_conflicts_[vertex_id] - 1);
                    number_of_conflicts_.set(edge.v, number_of_conflicts_[edge.v] - 1);
                }
                // Add new conflicts.
                if (color_id != -1 && color(edge.v) == color_id) {
                    total_number_of_conflicts_++;
                    conflicts_.insert(edge.e);
                    number_of_conflicts_.set(vertex_id, number_of_conflicts_[vertex_id] + 1);
                    number_of_conflicts_.set(edge.v, number_of_conflicts_[edge.v] + 1);
                }
            }
        } else {
            auto it = graph.neighbors_begin(vertex_id);
            auto it_end = graph.neighbors_end(vertex_id);
            for (; it != it_end; ++it) {
                // Remove old conflicts.
                if (contains(*it) && color(*it) == color(vertex_id)) {
                    total_number_of_conflicts_--;
                    number_of_conflicts_.set(vertex_id, number_of_conflicts_[vertex_id] - 1);
                    number_of_conflicts_.set(*it, number_of_conflicts_[*it] - 1);
                }
                // Add new conflicts.
                if (color_id != -1 && color(*it) == color_id) {
                    total_number_of_conflicts_++;
                    number_of_conflicts_.set(vertex_id, number_of_conflicts_[vertex_id] + 1);
                    number_of_conflicts_.set(*it, number_of_conflicts_[*it] + 1);
                }
            }
        }
    }
    // Update map_.
    if (color_id != -1) {
        map_.set(vertex_id, color_id);
    } else {
        map_.unset(vertex_id);
    }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Output ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * Output structure for a 'graphcoloring' problem.
 */
struct Output
{
    /** Constructor. */
    Output(
            const Instance& instance,
            optimizationtools::Info& info);

    /** Solution. */
    Solution solution;

    /** Lower bound. */
    ColorId lower_bound = 0;

    /** Elapsed time. */
    double time = -1;

    /** Return 'true' iff the solution is optimal. */
    bool optimal() const { return solution.feasible() && solution.number_of_colors() == lower_bound; }

    /** Get the upper bound. */
    ColorId upper_bound() const { return (solution.feasible())? solution.number_of_colors(): solution.instance().graph().maximum_degree() + 1; }

    /** Get the optimality gap. */
    double gap() const;

    /** Print current state. */
    void print(optimizationtools::Info& info, const std::stringstream& s) const;

    /** Update the solution. */
    void update_solution(
            const Solution& solution_new,
            const std::stringstream& s,
            optimizationtools::Info& info);

    /** Update the lower bound. */
    void update_lower_bound(
            ColorId lower_bound_new,
            const std::stringstream& s,
            optimizationtools::Info& info);

    /** Method to call at the end of the algorithm. */
    Output& algorithm_end(optimizationtools::Info& info);
};

}

