#pragma once

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/graph/abstract_graph.hpp"
#include "optimizationtools/graph/adjacency_list_graph.hpp"

#include <random>
#include <set>

namespace coloringsolver
{

using VertexId = optimizationtools::VertexId;
using VertexPos = optimizationtools::VertexPos;
using EdgeId = optimizationtools::EdgeId;
using Weight = optimizationtools::Weight;
using ColorId = int64_t; // c
using ColorPos = int64_t; // c_pos
using Penalty = int16_t; // p
using Counter = int64_t;
using Seed = int64_t;

class Instance
{

public:

    /*
     * Constructors and destructor.
     */

    /** Create an instance from a file. */
    Instance(std::string instance_path, std::string format);

    /** Create an instance manually. */
    Instance(VertexId number_of_vertices = 0);

    /** Add a vertex. */
    VertexId add_vertex(Weight weight = 1) { return adjacency_list_graph_->add_vertex(weight); }

    /** Set the weight of vertex 'v' to 'weight'. */
    void set_weight(VertexId v, Weight weight) { return adjacency_list_graph_->set_weight(v, weight); }

    /** Add an edge between vertex 'v1' and vertex 'v2'. */
    void add_edge(VertexId v1, VertexId v2) { adjacency_list_graph_->add_edge(v1, v2); }

    /** Set the weight of all vertices to 1. */
    void set_unweighted() { adjacency_list_graph_->set_unweighted(); }

    /** Remove duplicate edges (changes the edge ids). */
    void remove_duplicate_edges() { adjacency_list_graph_->remove_duplicate_edges(); }

    /**
     * Compute the core of the instance when looking for a coloration using 'k'
     * colors.
     *
     * Return the list of vertices that can be trivailly colored once the other
     * vertices have been colored. Thus, these vertices do not need to be
     * considered when looking for a k-coloration.
     */
    std::vector<VertexId> compute_core(ColorId k) const;

    /*
     * Getters.
     */

    /** Get graph. */
    inline const optimizationtools::AbstractGraph& graph() const { return *graph_; }

    /** Get the adjacency list graph. */
    inline const optimizationtools::AdjacencyListGraph* adjacency_list_graph() const { return adjacency_list_graph_; }

private:

    /*
     * Private attributes.
     */

    /** Graph. */
    std::unique_ptr<optimizationtools::AbstractGraph> graph_ = nullptr;

    /**
     * Adjacency list graph.
     *
     * 'nullptr' if 'graph_' is not an AdjacencyList graph.
     */
    optimizationtools::AdjacencyListGraph* adjacency_list_graph_ = nullptr;

};

/** Stream insertion operator. */
std::ostream& operator<<(std::ostream &os, const Instance& ins);

void init_display(
        const Instance& instance,
        optimizationtools::Info& info);

}

