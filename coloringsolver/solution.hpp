#pragma once

#include "coloringsolver/instance.hpp"

namespace coloringsolver
{

class Solution
{

public:

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

    /** Get the instance. */
    const Instance& instance() const { return instance_; }
    /** Return 'true' iff the solution is feasible. */
    bool feasible() const { return number_of_vertices() == instance().number_of_vertices() && conflicts_.size() == 0; };

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
    EdgePos number_of_conflicts() const { return conflicts_.size(); }

    /** Set color c to vertex v. */
    inline void set(VertexId v, ColorId c);

    Penalty penalty(EdgeId e) const { return penalties_[e]; }
    Penalty penalty() const { return penalty_; }
    std::vector<ColorId>::const_iterator colors_begin() const { return map().values_begin(); }
    std::vector<ColorId>::const_iterator colors_end()   const { return map().values_end(); }
    const optimizationtools::DoublyIndexedMap& map() const { return map_; }
    const optimizationtools::IndexedSet& conflicts() const { return conflicts_; }
    void increment_penalty(EdgeId e, Penalty p = 1);
    void set_penalty(EdgeId e, Penalty p);

    /** Write the solution to a file. */
    void write(std::string certificate_path) const;

private:

    /** Instance. */
    const Instance& instance_;
    /** Map storing the color assigned to each vertex. */
    optimizationtools::DoublyIndexedMap map_;
    /** Set of conflicting vertices. */
    optimizationtools::IndexedSet conflicts_;

    std::vector<Penalty> penalties_;
    Penalty penalty_ = 0;

};

std::ostream& operator<<(std::ostream& os, const Solution& solution);

void Solution::set(VertexId v, ColorId c)
{
    // Update conflicts_.
    for (const auto& edge: instance().vertex(v).edges) {
        if (color(edge.v) == color(v)) {
            conflicts_.remove(edge.e);
            penalty_ -= penalties_[edge.e];
        }
        if (color(edge.v) == c) {
            conflicts_.add(edge.e);
            penalty_ += penalties_[edge.e];
        }
    }
    map_.set(v, c);
}

/*********************************** Output ***********************************/

struct Output
{
    Output(
            const Instance& instance,
            optimizationtools::Info& info);

    Solution solution;
    ColorId lower_bound = 0;
    double time = -1;

    bool optimal() const { return solution.feasible() && solution.number_of_colors() == lower_bound; }
    ColorId upper_bound() const { return (solution.feasible())? solution.number_of_colors(): solution.instance().maximum_degree() + 1; }
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

