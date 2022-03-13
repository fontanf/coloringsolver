#include "coloringsolver/instance.hpp"

#include <sstream>
#include <iomanip>
#include <thread>

using namespace coloringsolver;

Instance::Instance(std::string instance_path, std::string format)
{
    std::ifstream file(instance_path);
    if (!file.good())
        throw std::runtime_error(
                "Unable to open file \"" + instance_path + "\".");

    if (format == "dimacs") {
        read_dimacs(file);
    } else if (format == "matrixmarket") {
        read_matrixmarket(file);
    } else if (format == "snap") {
        read_snap(file);
    } else {
        throw std::invalid_argument(
                "Unknown instance format \"" + format + "\".");
    }
}

Instance::Instance(VertexId number_of_vertices):
    vertices_(number_of_vertices)
{
    for (VertexId v = 0; v < number_of_vertices; ++v)
        vertices_[v].id = v;
}

void Instance::add_vertex()
{
    vertices_.push_back({});
    vertices_.back().id = vertices_.size() - 1;
}

void Instance::add_edge(VertexId v1, VertexId v2, bool check_duplicate)
{
    // Checks.
    check_vertex_index(v1);
    check_vertex_index(v2);

    if (v1 == v2) {
        //std::cerr << "\033[33m" << "WARNING, loop (" << v1 << ", " << v2 << ") ignored." << "\033[0m" << std::endl;
        return;
    }

    if (check_duplicate)
        for (const auto& edge: vertex(v1).edges)
            if (edge.v == v2)
                return;

    Edge e;
    e.id = edges_.size();
    e.v1 = v1;
    e.v2 = v2;
    edges_.push_back(e);

    VertexNeighbor vn1;
    vn1.e = e.id;
    vn1.v = v2;
    vertices_[v1].edges.push_back(vn1);
    if (maximum_degree_ < (VertexId)vertices_[v1].edges.size())
        maximum_degree_ = vertices_[v1].edges.size();

    VertexNeighbor vn2;
    vn2.e = e.id;
    vn2.v = v1;
    vertices_[v2].edges.push_back(vn2);
    if (maximum_degree_ < (VertexId)vertices_[v2].edges.size())
        maximum_degree_ = vertices_[v2].edges.size();
}

void Instance::read_dimacs(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;

    while (getline(file, tmp)) {
        line = optimizationtools::split(tmp, ' ');
        if (line.size() == 0) {
        } else if (line[0] == "c") {
            if (name_ == "")
                name_ = line.back();
        } else if (line[0] == "p") {
            VertexId number_of_vertices = stol(line[2]);
            vertices_.resize(number_of_vertices);
            for (VertexId v = 0; v < number_of_vertices; ++v)
                vertices_[v].id = v;
        } else if (line[0] == "e") {
            VertexId v1 = stol(line[1]) - 1;
            VertexId v2 = stol(line[2]) - 1;
            add_edge(v1, v2);
        }
    }
}

void Instance::read_matrixmarket(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    do {
        getline(file, tmp);
    } while (tmp[0] == '%');
    line = optimizationtools::split(tmp, ' ');
    VertexId n = stol(line[0]);
    vertices_.resize(n);

    while (getline(file, tmp)) {
        line = optimizationtools::split(tmp, ' ');
        VertexId v1 = stol(line[0]) - 1;
        VertexId v2 = stol(line[1]) - 1;
        add_edge(v1, v2);
    }
}

void Instance::read_snap(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    VertexId v1 = -1;
    VertexId v2 = -1;
    while (getline(file, tmp)) {
        if (tmp[0] == '#')
            continue;
        std::stringstream ss(tmp);
        ss >> v1 >> v2;
        if (std::max(v1, v2) >= number_of_vertices())
            vertices_.resize(std::max(v1, v2) + 1);
        add_edge(v1, v2);
    }
}

std::vector<VertexId> Instance::compute_core(ColorId k) const
{
    std::vector<VertexId> removed_vertices;
    std::vector<VertexPos> vertex_degrees(number_of_vertices(), -1);
    std::vector<VertexId> vertex_queue;
    for (VertexId v = 0; v < number_of_vertices(); ++v) {
        vertex_degrees[v] = degree(v);
        if (vertex_degrees[v] < k) {
            vertex_queue.push_back(v);
            removed_vertices.push_back(v);
        }
    }
    while (!vertex_queue.empty()) {
        VertexId v = vertex_queue.back();
        vertex_queue.pop_back();
        for (const auto& edge: vertex(v).edges) {
            if (vertex_degrees[edge.v] < k)
                continue;
            vertex_degrees[edge.v]--;
            if (vertex_degrees[edge.v] < k) {
                vertex_queue.push_back(edge.v);
                removed_vertices.push_back(edge.v);
            }
        }
    }
    //std::cout << "k " << k << " removed_vertices.size() " << removed_vertices.size() << std::endl;
    return removed_vertices;
}

void Instance::write(std::string instance_path, std::string format)
{
    std::ofstream file(instance_path);
    if (!file.good())
        throw std::runtime_error(
                "Unable to open file \"" + instance_path + "\".");

    if (format == "dimacs") {
        write_dimacs(file);
    } else if (format == "matrixmarket") {
        write_matrixmarket(file);
    } else if (format == "snap") {
        write_snap(file);
    } else {
        throw std::invalid_argument(
                "Unknown instance format \"" + format + "\".");
    }
}

void Instance::write_snap(std::ofstream& file)
{
    for (EdgeId e = 0; e < number_of_edges(); ++e)
        file << edge(e).v1 << " " << edge(e).v2 << std::endl;
}

void Instance::write_matrixmarket(std::ofstream& file)
{
    file << number_of_vertices()
        << " " << number_of_vertices()
        << " " << number_of_edges()
        << std::endl;
    for (EdgeId e = 0; e < number_of_edges(); ++e)
        file << edge(e).v1 + 1 << " " << edge(e).v2 + 1 << std::endl;
}

void Instance::write_dimacs(std::ofstream& file)
{
    file << "p edge " << number_of_vertices() << " " << number_of_edges() << std::endl;
    for (EdgeId e = 0; e < number_of_edges(); ++e)
        file << "e " << edge(e).v1 + 1 << " " << edge(e).v2 + 1 << std::endl;
}

