#include "coloringsolver/instance.hpp"

#include <iomanip>

using namespace coloringsolver;

namespace
{

optimizationtools::Graph read_graph(
        const std::string& instance_path,
        const std::string& format)
{
    if (format == "cliquegraph") {
        optimizationtools::CliqueGraphBuilder graph_builder;
        graph_builder.read(instance_path, format);
        return std::shared_ptr<const optimizationtools::CliqueGraph>(
                new optimizationtools::CliqueGraph(graph_builder.build()));
    } else {
        optimizationtools::AdjacencyListGraphBuilder graph_builder;
        graph_builder.read(instance_path, format);
        return std::shared_ptr<const optimizationtools::AdjacencyListGraph>(
                new optimizationtools::AdjacencyListGraph(graph_builder.build()));
    }
}

template <typename Graph>
void format_edges(
        const Graph& graph,
        std::ostream& os)
{
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        auto it = graph.neighbors_begin(vertex_id);
        auto it_end = graph.neighbors_end(vertex_id);
        for (; it != it_end; ++it) {
            if (vertex_id < *it) {
                os
                    << std::setw(12) << vertex_id
                    << std::setw(12) << *it
                    << std::endl;
            }
        }
    }
}

template <typename Graph>
std::vector<VertexId> compute_core(
        const Graph& graph,
        ColorId k)
{
    std::vector<VertexId> removed_vertices;
    std::vector<VertexPos> vertex_degrees(graph.number_of_vertices(), -1);
    std::vector<VertexId> vertex_queue;
    for (VertexId vertex_id = 0;
            vertex_id < graph.number_of_vertices();
            ++vertex_id) {
        vertex_degrees[vertex_id] = graph.degree(vertex_id);
        if (vertex_degrees[vertex_id] < k)
            vertex_queue.push_back(vertex_id);
    }
    while (!vertex_queue.empty()) {
        VertexId vertex_id = vertex_queue.back();
        removed_vertices.push_back(vertex_id);
        vertex_queue.pop_back();
        for (auto it = graph.neighbors_begin(vertex_id);
                it != graph.neighbors_end(vertex_id);
                ++it) {
            if (vertex_degrees[*it] < k)
                continue;
            vertex_degrees[*it]--;
            if (vertex_degrees[*it] < k)
                vertex_queue.push_back(*it);
        }
    }
    //std::cout << "k " << k << " removed_vertices.size() " << removed_vertices.size() << std::endl;
    return removed_vertices;
}

}

Instance::Instance(
        const std::string& instance_path,
        const std::string& format):
    graph_(read_graph(instance_path, format)) { }

Instance::Instance(const optimizationtools::Graph& graph):
    graph_(graph) { }

std::ostream& Instance::format(
        std::ostream& os,
        int verbosity_level) const
{
    if (verbosity_level >= 1) {
        os
            << "Number of vertices:  " << graph().number_of_vertices() << std::endl
            << "Number of edges:     " << graph().number_of_edges() << std::endl
            << "Density:             " << graph().density() << std::endl
            << "Average degree:      " << graph().average_degree() << std::endl
            << "Highest degree:      " << graph().highest_degree() << std::endl
            ;
    }

    if (verbosity_level >= 2) {
        os << std::endl
            << std::setw(12) << "VertexId"
            << std::setw(12) << "Weight"
            << std::setw(12) << "Degree"
            << std::endl
            << std::setw(12) << "--------"
            << std::setw(12) << "------"
            << std::setw(12) << "------"
            << std::endl;
        for (VertexId vertex_id = 0;
                vertex_id < graph().number_of_vertices();
                ++vertex_id) {
            os
                << std::setw(12) << vertex_id
                << std::setw(12) << graph().weight(vertex_id)
                << std::setw(12) << graph().degree(vertex_id)
                << std::endl;
        }
    }

    if (verbosity_level >= 3) {
        os << std::endl
            << std::setw(12) << "Edge"
            << std::setw(12) << "Vertex 1"
            << std::setw(12) << "Vertex 2"
            << std::endl
            << std::setw(12) << "----"
            << std::setw(12) << "--------"
            << std::setw(12) << "--------"
            << std::endl;
        FUNCTION_WITH_GRAPH(format_edges, graph(), os);
    }

    return os;
}

std::vector<VertexId> Instance::compute_core(ColorId k) const
{
    return FUNCTION_WITH_GRAPH(::compute_core, graph(), k);
}
