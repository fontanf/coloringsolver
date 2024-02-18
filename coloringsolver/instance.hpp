#pragma once

#include "optimizationtools/graph/abstract_graph.hpp"
#include "optimizationtools/graph/adjacency_list_graph.hpp"

#include <memory>

namespace coloringsolver
{

using VertexId = optimizationtools::VertexId;
using VertexPos = optimizationtools::VertexPos;
using EdgeId = optimizationtools::EdgeId;
using ColorId = int64_t; // c
using ColorPos = int64_t; // c_pos
using Penalty = int16_t; // p
using Counter = int64_t;
using Seed = int64_t;

/**
 * Instance class for a graph coloring problem.
 */
class Instance
{

public:

    /*
     * Constructors and destructor
     */

    /** Create an instance from an AbstrctGraph. */
    Instance(const std::shared_ptr<const optimizationtools::AbstractGraph>& abstract_graph);

    /** Create an instance from a file. */
    Instance(
            const std::string& instance_path,
            const std::string& format);

    /*
     * Getters
     */

    /** Get graph. */
    inline const optimizationtools::AbstractGraph& graph() const { return *graph_; }

    /** Get the adjacency list graph. */
    inline const optimizationtools::AdjacencyListGraph* adjacency_list_graph() const { return adjacency_list_graph_; }

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
     * Export
     */

    /** Print the instance. */
    std::ostream& format(
            std::ostream& os,
            int verbosity_level = 1) const;

private:

    /*
     * Private attributes
     */

    /** Graph. */
    std::shared_ptr<const optimizationtools::AbstractGraph> graph_ = nullptr;

    /**
     * Adjacency list graph.
     *
     * 'nullptr' if 'graph_' is not an AdjacencyList graph.
     */
    const optimizationtools::AdjacencyListGraph* adjacency_list_graph_ = nullptr;

};

}
