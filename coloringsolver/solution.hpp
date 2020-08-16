#pragma once

#include "coloringsolver/instance.hpp"

namespace coloringsolver
{

using optimizationtools::Info;

class Solution
{

public:

    Solution(const Instance& instance);
    Solution(const Instance& instance, std::string filepath);
    Solution(const Solution& solution);
    Solution& operator=(const Solution& solution);
    virtual ~Solution() { }

    const Instance& instance() const { return instance_; }
    bool feasible() const { return vertex_number() == instance().vertex_number() && conflicts_.size() == 0; };

    ColorId color_number() const { return map_.value_number(); }
    bool contains(VertexId v) const { return map_.contains(v); }
    ColorId color(VertexId v) const { return map_[v]; }
    VertexPos vertex_number() const { return map_.element_number(); }
    VertexPos vertex_number(ColorId c) const { return map_.element_number(c); }

    const optimizationtools::DoublyIndexedMap& map() const { return map_; }

    inline void set(VertexId v, ColorId c);

    void increment_penalty(EdgeId e, Penalty p = 1);
    void set_penalty(EdgeId e, Penalty p);

    void write(std::string filepath) const;

private:

    const Instance& instance_;
    optimizationtools::DoublyIndexedMap map_;
    optimizationtools::IndexedSet conflicts_;
    std::vector<Penalty> penalties_;
    Penalty penalty_ = 0;

};

std::ostream& operator<<(std::ostream& os, const Solution& solution);

void Solution::set(VertexId v, ColorId c)
{
    // Update conflicts_.
    for (const auto& vn: instance().vertex(v).edges) {
        if (color(v) == color(vn.v)) {
            conflicts_.remove(vn.e);
            penalty_ -= penalties_[vn.e];
        }
        if (color(v) == c) {
            conflicts_.add(vn.e);
            penalty_ += penalties_[vn.e];
        }
    }
    map_.set(v, c);
}

/*********************************** Output ***********************************/

struct Output
{
    Output(const Instance& instance, Info& info);
    Solution solution;
    ColorId lower_bound = 0;
    double time = -1;

    bool optimal() const { return solution.feasible() && solution.color_number() == lower_bound; }
    ColorId upper_bound() const { return (solution.feasible())? solution.color_number(): solution.instance().degree_max() + 1; }
    double gap() const;
    void print(Info& info, const std::stringstream& s) const;

    void update_solution(const Solution& solution_new, const std::stringstream& s, Info& info);
    void update_lower_bound(ColorId lower_bound_new, const std::stringstream& s, Info& info);

    Output& algorithm_end(Info& info);
};

ColorId algorithm_end(ColorId lower_bound, Info& info);

}

