#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_map.hpp"
#include "optimizationtools/doubly_indexed_map.hpp"

#include <random>
#include <set>

namespace coloringsolver
{

typedef int32_t VertexId; // v
typedef int32_t VertexPos; // v_pos
typedef int64_t EdgeId; // e
typedef int64_t EdgePos; // e_pos
typedef int64_t ColorId; // c
typedef int64_t ColorPos; // c_pos
typedef int16_t Penalty; // p
typedef int64_t Counter;
typedef int64_t Seed;

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
    void add_edge(VertexId v1, VertexId v2, bool check_duplicate = true);

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

};

std::ostream& operator<<(std::ostream &os, const Instance& ins);

}

