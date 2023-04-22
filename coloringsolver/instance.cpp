#include "coloringsolver/instance.hpp"

#include "optimizationtools/graph/clique_graph.hpp"

#include <sstream>
#include <iomanip>
#include <thread>

using namespace coloringsolver;

Instance::Instance(std::string instance_path, std::string format)
{
    if (format == "cliquegraph") {
        graph_ = std::unique_ptr<optimizationtools::AbstractGraph>(
                    new optimizationtools::CliqueGraph(instance_path, format));
    } else {
        graph_ = std::unique_ptr<optimizationtools::AbstractGraph>(
                    new optimizationtools::AdjacencyListGraph(instance_path, format));
        adjacency_list_graph_ = static_cast<optimizationtools::AdjacencyListGraph*>(graph_.get());
    }
}

Instance::Instance(
        VertexId number_of_vertices):
    graph_(std::unique_ptr<optimizationtools::AbstractGraph>(
                new optimizationtools::AdjacencyListGraph(number_of_vertices))),
    adjacency_list_graph_(static_cast<optimizationtools::AdjacencyListGraph*>(graph_.get())) { }

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

void coloringsolver::init_display(
        const Instance& instance,
        optimizationtools::Info& info)
{
    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();
    EdgeId m = graph.number_of_edges();
    info.os()
        << "=====================================" << std::endl
        << "           Coloring Solver           " << std::endl
        << "=====================================" << std::endl
        << std::endl
        << "Instance" << std::endl
        << "--------" << std::endl
        << "Number of vertices:  " << n << std::endl
        << "Number of edges:     " << m << std::endl
        << "Density:             " << (double)m * 2 / n / (n - 1) << std::endl
        << "Average degree:      " << (double)graph.number_of_edges() * 2 / n << std::endl
        << "Maximum degree:      " << graph.maximum_degree() << std::endl
        << std::endl;
}

