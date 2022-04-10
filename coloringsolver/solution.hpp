#pragma once

#include "coloringsolver/instance.hpp"

#include "optimizationtools/containers/indexed_map.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"

#include <unordered_set>

namespace coloringsolver
{

class Solution
{

public:

    /*
     * Constructors and destructor.
     */

    /** Create an empty solution. */
    Solution(const Instance& instance);
    /** Create a solution from a certificate file. */
    Solution(const Instance& instance, std::string certificate_path);
    /** Copy constructor. */
    Solution(const Solution& solution);
    /** Copy assignment operator. */
    Solution& operator=(const Solution& solution);
    /** Destructor. */
    virtual ~Solution() { }

    /*
     * Getters.
     */

    /** Get the instance. */
    const Instance& instance() const { return instance_; }
    /** Return 'true' iff the solution is feasible. */
    bool feasible() const { return number_of_vertices() == instance().graph()->number_of_vertices() && number_of_conflicts() == 0; };
    /** Get the number of colors used in the solution. */
    ColorId number_of_colors() const { return map_.number_of_values(); }
    /** Return 'true' iff a color is assigned to vertex v in the solution. */
    bool contains(VertexId v) const { return map_.contains(v); }
    /** Get the color of vertex v. */
    ColorId color(VertexId v) const { return map_[v]; }
    /** Get the number of vertices with an assigned color. */
    VertexPos number_of_vertices() const { return map_.number_of_elements(); }
    /** Get the number of vertices with color c. */
    VertexPos number_of_vertices(ColorId c) const { return map_.number_of_elements(c); }
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
     * Setters.
     */

    /** Set color c to vertex v. */
    inline void set(VertexId v, ColorId c);

    /*
     * Export.
     */

    /** Write the solution to a file. */
    void write(std::string certificate_path) const;

private:

    /*
     * Private attributes.
     */

    /** Instance. */
    const Instance& instance_;
    /** Map storing the color assigned to each vertex. */
    optimizationtools::DoublyIndexedMap map_;
    /** Set of conflicting edges. */
    std::unordered_set<EdgeId> conflicts_;
    /** Conflicting vertices. */
    optimizationtools::IndexedMap<VertexPos> number_of_conflicts_;
    /** Number of conflicts. */
    EdgeId total_number_of_conflicts_ = 0;

};

std::ostream& operator<<(std::ostream& os, const Solution& solution);

void Solution::set(VertexId v, ColorId c)
{
    const optimizationtools::AbstractGraph* graph = instance().graph();

    // Checks.
    graph->check_vertex_index(v);
    if (c < -1 || c >= graph->number_of_vertices()) {
        throw std::out_of_range(
                "Invalid color value: \"" + std::to_string(c) + "\"."
                + " Color values should belong to [-1, "
                + std::to_string(number_of_vertices() - 1) + "].");
    }

    // Update conflicts_.
    if (instance().adjacency_list_graph() != nullptr) {
        for (EdgeId e: instance().adjacency_list_graph()->edges(v)) {
            VertexId v_neighbor = instance().adjacency_list_graph()->other_end(e, v);
            // Remove old conflicts.
            if (contains(v_neighbor) && color(v_neighbor) == color(v)) {
                total_number_of_conflicts_--;
                conflicts_.erase(e);
                number_of_conflicts_.set(v, number_of_conflicts_[v] - 1);
                number_of_conflicts_.set(v_neighbor, number_of_conflicts_[v_neighbor] - 1);
            }
            // Add new conflicts.
            if (c != -1 && color(v_neighbor) == c) {
                total_number_of_conflicts_++;
                conflicts_.insert(e);
                number_of_conflicts_.set(v, number_of_conflicts_[v] + 1);
                number_of_conflicts_.set(v_neighbor, number_of_conflicts_[v_neighbor] + 1);
            }
        }
    } else {
        for (auto it = graph->neighbors_begin(v);
                it != graph->neighbors_end(v); ++it) {
            // Remove old conflicts.
            if (contains(*it) && color(*it) == color(v)) {
                total_number_of_conflicts_--;
                number_of_conflicts_.set(v, number_of_conflicts_[v] - 1);
                number_of_conflicts_.set(*it, number_of_conflicts_[*it] - 1);
            }
            // Add new conflicts.
            if (c != -1 && color(*it) == c) {
                total_number_of_conflicts_++;
                number_of_conflicts_.set(v, number_of_conflicts_[v] + 1);
                number_of_conflicts_.set(*it, number_of_conflicts_[*it] + 1);
            }
        }
    }
    // Update map_.
    if (c != -1) {
        map_.set(v, c);
    } else {
        map_.unset(v);
    }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Output ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct Output
{
    Output(
            const Instance& instance,
            optimizationtools::Info& info);

    Solution solution;
    ColorId lower_bound = 0;
    double time = -1;

    bool optimal() const { return solution.feasible() && solution.number_of_colors() == lower_bound; }
    ColorId upper_bound() const { return (solution.feasible())? solution.number_of_colors(): solution.instance().graph()->maximum_degree() + 1; }
    double gap() const;
    void print(optimizationtools::Info& info, const std::stringstream& s) const;

    void update_solution(
            const Solution& solution_new,
            const std::stringstream& s,
            optimizationtools::Info& info);

    void update_lower_bound(
            ColorId lower_bound_new,
            const std::stringstream& s,
            optimizationtools::Info& info);

    Output& algorithm_end(optimizationtools::Info& info);
};

ColorId algorithm_end(
        ColorId lower_bound,
        optimizationtools::Info& info);

}

