#pragma once

#include "coloringsolver/instance.hpp"

#include "optimizationtools/utils/output.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/indexed_map.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"

#include <unordered_set>
#include <iomanip>

namespace coloringsolver
{

/**
 * Solution class for a graph coloring problem.
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
            const std::string& certificate_path);

    /*
     * Getters
     */

    /** Get the instance. */
    const Instance& instance() const { return *instance_; }

    /** Return 'true' iff the solution is feasible. */
    bool feasible() const { return number_of_vertices() == instance().graph().number_of_vertices() && number_of_conflicts() == 0; };

    /** Get the number of colors used in the solution. */
    ColorId number_of_colors() const { return map_.number_of_values(); }

    /** Get the number of colors used in the solution. */
    ColorId objective_value() const { return number_of_colors(); }

    /** Return 'true' iff a color is assigned to vertex v in the solution. */
    bool contains(VertexId vertex_id) const { return map_.contains(vertex_id); }

    /** Get the color of a vertex. */
    ColorId color(VertexId vertex_id) const { return map_[vertex_id]; }

    /** Get the number of vertices with an assigned color. */
    VertexPos number_of_vertices() const { return map_.number_of_elements(); }

    /** Get the number of vertices with a given color. */
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

    /** Print the instance. */
    std::ostream& format(
            std::ostream& os,
            int verbosity_level = 1) const;

    /** Write the solution to a file. */
    void write(const std::string& certificate_path) const;

    /** Export solution characteristics to a JSON structure. */
    nlohmann::json to_json() const;

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
                if (contains(edge.vertex_id)
                        && color(edge.vertex_id) == color(vertex_id)) {
                    total_number_of_conflicts_--;
                    conflicts_.erase(edge.edge_id);
                    number_of_conflicts_.set(
                            vertex_id,
                            number_of_conflicts_[vertex_id] - 1);
                    number_of_conflicts_.set(
                            edge.vertex_id,
                            number_of_conflicts_[edge.vertex_id] - 1);
                }
                // Add new conflicts.
                if (color_id != -1
                        && color(edge.vertex_id) == color_id) {
                    total_number_of_conflicts_++;
                    conflicts_.insert(edge.edge_id);
                    number_of_conflicts_.set(
                            vertex_id,
                            number_of_conflicts_[vertex_id] + 1);
                    number_of_conflicts_.set(
                            edge.vertex_id,
                            number_of_conflicts_[edge.vertex_id] + 1);
                }
            }
        } else {
            auto it = graph.neighbors_begin(vertex_id);
            auto it_end = graph.neighbors_end(vertex_id);
            for (; it != it_end; ++it) {
                // Remove old conflicts.
                if (contains(*it)
                        && color(*it) == color(vertex_id)) {
                    total_number_of_conflicts_--;
                    number_of_conflicts_.set(
                            vertex_id,
                            number_of_conflicts_[vertex_id] - 1);
                    number_of_conflicts_.set(
                            *it,
                            number_of_conflicts_[*it] - 1);
                }
                // Add new conflicts.
                if (color_id != -1
                        && color(*it) == color_id) {
                    total_number_of_conflicts_++;
                    number_of_conflicts_.set(
                            vertex_id,
                            number_of_conflicts_[vertex_id] + 1);
                    number_of_conflicts_.set(
                            *it,
                            number_of_conflicts_[*it] + 1);
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
 * Output structure for a graph coloring problem.
 */
inline optimizationtools::ObjectiveDirection objective_direction()
{
    return optimizationtools::ObjectiveDirection::Minimize;
}

/**
 * Output structure for a set covering problem.
 */
struct Output: optimizationtools::Output
{
    /** Constructor. */
    Output(const Instance& instance):
        solution(instance) { }


    /** Solution. */
    Solution solution;

    /** Bound. */
    ColorId bound = 0;

    /** Elapsed time. */
    double time = 0.0;


    std::string solution_value() const
    {
        return optimizationtools::solution_value(
            objective_direction(),
            solution.feasible(),
            solution.objective_value());
    }

    double absolute_optimality_gap() const
    {
        return optimizationtools::absolute_optimality_gap(
                objective_direction(),
                solution.feasible(),
                solution.objective_value(),
                bound);
    }

    double relative_optimality_gap() const
    {
       return optimizationtools::relative_optimality_gap(
            objective_direction(),
            solution.feasible(),
            solution.objective_value(),
            bound);
    }

    virtual nlohmann::json to_json() const
    {
        return nlohmann::json {
            {"Solution", solution.to_json()},
            {"Value", solution_value()},
            {"Bound", bound},
            {"AbsoluteOptimalityGap", absolute_optimality_gap()},
            {"RelativeOptimalityGap", relative_optimality_gap()},
            {"Time", time}
        };
    }

    virtual int format_width() const { return 30; }

    virtual void format(std::ostream& os) const
    {
        int width = format_width();
        os
            << std::setw(width) << std::left << "Value: " << solution_value() << std::endl
            << std::setw(width) << std::left << "Bound: " << bound << std::endl
            << std::setw(width) << std::left << "Absolute optimality gap: " << absolute_optimality_gap() << std::endl
            << std::setw(width) << std::left << "Relative optimality gap (%): " << relative_optimality_gap() * 100 << std::endl
            << std::setw(width) << std::left << "Time (s): " << time << std::endl
            ;
    }
};

using NewSolutionCallback = std::function<void(const Output&, const std::string&)>;

struct Parameters: optimizationtools::Parameters
{
    /** Callback function called when a new best solution is found. */
    NewSolutionCallback new_solution_callback = [](const Output&, const std::string&) { };


    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = optimizationtools::Parameters::to_json();
        json.merge_patch({});
        return json;
    }

    virtual int format_width() const override { return 23; }

    virtual void format(std::ostream& os) const override
    {
        optimizationtools::Parameters::format(os);
        //int width = format_width();
        //os
        //    ;
    }
};

}
