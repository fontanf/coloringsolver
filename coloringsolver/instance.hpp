#pragma once

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/indexed_map.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"

#include <random>
#include <set>

namespace coloringsolver
{

using VertexId = int32_t; // v
using VertexPos = int32_t; // v_pos
using EdgeId = int64_t; // e
using EdgePos = int64_t; // e_pos
using ColorId = int64_t; // c
using ColorPos = int64_t; // c_pos
using Penalty = int16_t; // p
using Counter = int64_t;
using Seed = int64_t;

struct VertexNeighbor
{
    EdgeId e;
    VertexId v;
};

struct Vertex
{
    VertexId id;
    std::vector<VertexNeighbor> edges;
};

struct Edge
{
    EdgeId id;
    VertexId v1;
    VertexId v2;
};

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
    /** Set the name of the instance. */
    void set_name(std::string name) { name_ = name; }
    /** Add a vertex. */
    void add_vertex();
    /** Add an edge between vertex v1 and vertex v2. */
    void add_edge(VertexId v1, VertexId v2);
    /** Clear the instance (remove all vertices and edges). */
    void clear();
    /** Clear the edges of the instance (remove all edges). */
    void clear_edges();
    /** Remove duplicate edges (changes the edge ids). */
    void remove_duplicate_edges();

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

    /** Get the name of the instance. */
    inline std::string name() const { return name_; }
    /** Get the number of vertices. */
    inline VertexId number_of_vertices() const { return vertices_.size(); }
    /** Get the number of edges. */
    inline EdgeId number_of_edges() const { return edges_.size(); }
    /** Get vertex v. */
    inline const Vertex& vertex(VertexId v) const { return vertices_[v]; }
    /** Get edge e. */
    inline const Edge& edge(EdgeId e) const { return edges_[e]; }
    /** Get the degree of vertex v. */
    inline VertexId degree(VertexId v) const { return vertices_[v].edges.size(); }
    /** Get the maximum degree of the instance. */
    inline VertexId maximum_degree() const { return maximum_degree_; }

    /*
     * Export.
     */

    /** Write the instance to a file. */
    void write(std::string instance_path, std::string format);

    /*
     * Checkers.
     */

    /** Check if vertex index 'v' is within the correct range. */
    inline void check_vertex_index(VertexId v) const
    {
        if (v < 0 || v >= number_of_vertices())
            throw std::out_of_range(
                    "Invalid vertex index: \"" + std::to_string(v) + "\"."
                    + " Vertex indices should belong to [0, "
                    + std::to_string(number_of_vertices() - 1) + "].");
    }

private:

    /*
     * Private attributes.
     */

    /** Name of the instance. */
    std::string name_ = "";
    /** Vertices. */
    std::vector<Vertex> vertices_;
    /** Edges. */
    std::vector<Edge> edges_;
    /** Maximum degree of the instance. */
    VertexId maximum_degree_ = 0;

    /*
     * Private methods.
     */

    /** Read an instance in 'dimacs' format. */
    void read_dimacs(std::ifstream& file);
    /** Read an instance in 'matrixmarket' format. */
    void read_matrixmarket(std::ifstream& file);
    /** Read an instance in 'snap' format. */
    void read_snap(std::ifstream& file);
    /** Read an instance in 'dimacs2010' format. */
    void read_dimacs2010(std::ifstream& file);
    /** Write the instance in 'dimacs' format. */
    void write_dimacs(std::ofstream& file);
    /** Write the instance in 'matrixmarket' format. */
    void write_matrixmarket(std::ofstream& file);
    /** Write the instance in 'snap' format. */
    void write_snap(std::ofstream& file);

};

std::ostream& operator<<(std::ostream &os, const Instance& ins);

void init_display(
        const Instance& instance,
        optimizationtools::Info& info);

}

