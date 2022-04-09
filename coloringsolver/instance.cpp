#include "coloringsolver/instance.hpp"

#include <sstream>
#include <iomanip>
#include <thread>

using namespace coloringsolver;

Instance::Instance(std::string instance_path, std::string format)
{
    std::ifstream file(instance_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + instance_path + "\".");
    }

    set_name(instance_path);
    if (format == "dimacs") {
        read_dimacs(file);
    } else if (format == "matrixmarket") {
        read_matrixmarket(file);
    } else if (format == "snap") {
        read_snap(file);
    } else if (format == "dimacs2010") {
        read_dimacs2010(file);
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

void Instance::add_edge(VertexId v1, VertexId v2)
{
    // Checks.
    check_vertex_index(v1);
    check_vertex_index(v2);

    if (v1 == v2) {
        //std::cerr << "\033[33m" << "WARNING, loop (" << v1 << ", " << v2 << ") ignored." << "\033[0m" << std::endl;
        return;
    }

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
            for (VertexId v = 0; v < number_of_vertices; ++v)
                add_vertex();
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
    std::stringstream ss(tmp);
    VertexId n = -1;
    ss >> n;
    for (VertexId v = 0; v < n; ++v)
        add_vertex();

    VertexId v1 = -1;
    VertexId v2 = -1;
    while (getline(file, tmp)) {
        std::stringstream ss(tmp);
        ss >> v1 >> v2;
        add_edge(v1 - 1, v2 - 1);
    }
}

void Instance::read_snap(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    do {
        getline(file, tmp);
    } while (tmp[0] == '#');

    VertexId v1 = -1;
    VertexId v2 = -1;
    for (;;) {
        file >> v1 >> v2;
        if (file.eof())
            break;
        while (std::max(v1, v2) >= number_of_vertices())
            add_vertex();
        add_edge(v1, v2);
    }
}

void Instance::read_dimacs2010(std::ifstream& file)
{
    std::string tmp;
    std::vector<std::string> line;
    bool first = true;
    VertexId v = -1;
    while (v != number_of_vertices()) {
        getline(file, tmp);
        //std::cout << tmp << std::endl;
        line = optimizationtools::split(tmp, ' ');
        if (tmp[0] == '%')
            continue;
        if (first) {
            VertexId n = stol(line[0]);
            for (VertexId v = 0; v < n; ++v)
                add_vertex();
            first = false;
            v = 0;
        } else {
            for (std::string str: line) {
                VertexId v2 = stol(str) - 1;
                if (v2 > v)
                    add_edge(v, v2);
            }
            v++;
        }
    }
}

void Instance::clear()
{
    vertices_.clear();
    edges_.clear();
    maximum_degree_ = 0;
}

void Instance::clear_edges()
{
    edges_.clear();
    maximum_degree_ = 0;
    for (VertexId v = 0; v < number_of_vertices(); ++v)
        vertices_[v].edges.clear();
}

void Instance::remove_duplicate_edges()
{
    std::vector<std::vector<VertexId>> neighbors(number_of_vertices());
    for (VertexId v = 0; v < number_of_vertices(); ++v) {
        for (auto& edge: vertices_[v].edges)
            if (edge.v > v)
                neighbors[v].push_back(edge.v);
        sort(neighbors[v].begin(), neighbors[v].end());
        neighbors[v].erase(
                std::unique(
                    neighbors[v].begin(),
                    neighbors[v].end()),
                neighbors[v].end());
    }
    clear_edges();
    for (VertexId v1 = 0; v1 < number_of_vertices(); ++v1)
        for (VertexId v2: neighbors[v1])
            add_edge(v1, v2);
}

std::vector<VertexId> Instance::compute_core(ColorId k) const
{
    std::vector<VertexId> removed_vertices;
    std::vector<VertexPos> vertex_degrees(number_of_vertices(), -1);
    std::vector<VertexId> vertex_queue;
    for (VertexId v = 0; v < number_of_vertices(); ++v) {
        vertex_degrees[v] = degree(v);
        if (vertex_degrees[v] < k)
            vertex_queue.push_back(v);
    }
    while (!vertex_queue.empty()) {
        VertexId v = vertex_queue.back();
        removed_vertices.push_back(v);
        vertex_queue.pop_back();
        for (const auto& edge: vertex(v).edges) {
            if (vertex_degrees[edge.v] < k)
                continue;
            vertex_degrees[edge.v]--;
            if (vertex_degrees[edge.v] < k)
                vertex_queue.push_back(edge.v);
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

void coloringsolver::init_display(
        const Instance& instance,
        optimizationtools::Info& info)
{
    VertexId n = instance.number_of_vertices();
    EdgeId m = instance.number_of_edges();
    FFOT_VER(info,
               "=====================================" << std::endl
            << "           Coloring Solver           " << std::endl
            << "=====================================" << std::endl
            << std::endl
            << "Instance" << std::endl
            << "--------" << std::endl
            << "Name:                " << instance.name() << std::endl
            << "Number of vertices:  " << n << std::endl
            << "Number of edges:     " << m << std::endl
            << "Density:             " << (double)m * 2 / n / (n - 1) << std::endl
            << "Average degree:      " << (double)instance.number_of_edges() * 2 / n << std::endl
            << "Maximum degree:      " << instance.maximum_degree() << std::endl
            << std::endl);
}

