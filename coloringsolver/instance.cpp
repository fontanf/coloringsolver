#include "coloringsolver/instance.hpp"

#include "optimizationtools/graph/clique_graph.hpp"

#include <iomanip>

using namespace coloringsolver;

Instance::Instance(
        const std::string& instance_path,
        const std::string& format)
{
    if (format == "cliquegraph") {
        optimizationtools::CliqueGraphBuilder graph_builder;
        graph_builder.read(instance_path, format);
        graph_ = std::shared_ptr<const optimizationtools::AbstractGraph>(
                new optimizationtools::CliqueGraph(graph_builder.build()));
    } else {
        optimizationtools::AdjacencyListGraphBuilder graph_builder;
        graph_builder.read(instance_path, format);
        graph_ = std::shared_ptr<const optimizationtools::AbstractGraph>(
                new optimizationtools::AdjacencyListGraph(graph_builder.build()));
        adjacency_list_graph_ = static_cast<const optimizationtools::AdjacencyListGraph*>(graph_.get());
    }
}

Instance::Instance(const std::shared_ptr<const optimizationtools::AbstractGraph>& abstract_graph):
    graph_(abstract_graph),
    adjacency_list_graph_(dynamic_cast<const optimizationtools::AdjacencyListGraph*>(graph_.get())) { }

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
        for (VertexId vertex_id = 0;
                vertex_id < graph().number_of_vertices();
                ++vertex_id) {
            auto it = graph().neighbors_begin(vertex_id);
            auto it_end = graph().neighbors_end(vertex_id);
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

    return os;
}

std::vector<VertexId> Instance::compute_core(ColorId k) const
{
    std::vector<VertexId> removed_vertices;
    std::vector<VertexPos> vertex_degrees(graph().number_of_vertices(), -1);
    std::vector<VertexId> vertex_queue;
    for (VertexId vertex_id = 0;
            vertex_id < graph().number_of_vertices();
            ++vertex_id) {
        vertex_degrees[vertex_id] = graph().degree(vertex_id);
        if (vertex_degrees[vertex_id] < k)
            vertex_queue.push_back(vertex_id);
    }
    while (!vertex_queue.empty()) {
        VertexId vertex_id = vertex_queue.back();
        removed_vertices.push_back(vertex_id);
        vertex_queue.pop_back();
        for (auto it = graph().neighbors_begin(vertex_id);
                it != graph().neighbors_end(vertex_id);
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
