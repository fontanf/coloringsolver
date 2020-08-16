#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_map.hpp"
#include "optimizationtools/doubly_indexed_map.hpp"

#include <random>
#include <set>

#define TOL 0.0000001

namespace coloringsolver
{

using optimizationtools::Info;

typedef int64_t VertexId; // v
typedef int64_t VertexPos; // v_pos
typedef int64_t EdgeId; // e
typedef int64_t EdgePos; // e_pos
typedef int64_t ComponentId; // c
typedef int64_t ColorId; // c
typedef int64_t ColorPos; // c_pos
typedef int64_t Penalty; // p
typedef int64_t Counter;
typedef int64_t Seed;

class Solution;

/******************************************************************************/

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

    /** Constructor from file. */
    Instance(std::string filepath, std::string format);

    /** Manual constructor. */
    Instance(VertexId vertex_number);
    void set_name(std::string name) { name_ = name; }
    void add_edge(VertexId v1, VertexId v2);

    /** Getters. */
    inline std::string name() const { return name_; }
    inline VertexId  vertex_number() const { return vertices_.size(); }
    inline EdgeId      edge_number() const { return edges_.size(); }
    inline const Vertex& vertex(VertexId id) const { return vertices_[id]; }
    inline const Edge&       edge(EdgeId id) const { return edges_[id]; }
    inline VertexId degree(VertexId v) const { return vertices_[v].edges.size(); }
    inline VertexId degree_max() const { return degree_max_; }

    /** Export. */
    void write(std::string filepath, std::string format);

private:

    /**
     * Attributes.
     */

    std::string name_ = "";
    std::vector<Vertex> vertices_;
    std::vector<Edge> edges_;
    VertexId degree_max_ = 0;

    /**
     * Private methods.
     */

    void read_dimacs(std::ifstream& file);

};

std::ostream& operator<<(std::ostream &os, const Instance& ins);

}

