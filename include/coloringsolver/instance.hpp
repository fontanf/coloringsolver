#pragma once

#include "optimizationtools/graph/graph.hpp"

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

    /** Create an instance from a graph. */
    Instance(const optimizationtools::Graph& graph);

    /**
     * Create an instance from a file.
     *
     * The graph is a CliqueGraph for the 'cliquegraph' format, an
     * AdjacencyListGraph otherwise.
     */
    Instance(
            const std::string& instance_path,
            const std::string& format);

    /*
     * Getters
     */

    /** Get graph. */
    inline const optimizationtools::Graph& graph() const { return graph_; }

    /**
     * Get the adjacency list graph.
     *
     * 'nullptr' if the graph is not an AdjacencyListGraph.
     */
    inline const optimizationtools::AdjacencyListGraph* adjacency_list_graph() const { return graph_.adjacency_list_graph(); }

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
    optimizationtools::Graph graph_;

};

}
