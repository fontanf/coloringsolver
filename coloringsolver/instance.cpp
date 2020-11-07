#include "coloringsolver/instance.hpp"

#include <sstream>
#include <iomanip>
#include <thread>

using namespace coloringsolver;

Instance::Instance(std::string filepath, std::string format)
{
    std::ifstream file(filepath);
    if (!file.good()) {
        std::cerr << "\033[31m" << "ERROR, unable to open file \"" << filepath << "\"" << "\033[0m" << std::endl;
        assert(false);
        return;
    }

    if (format == "dimacs") {
        read_dimacs(file);
    } else if (format == "matrixmarket") {
        read_matrixmarket(file);
    } else {
        std::cerr << "\033[31m" << "ERROR, unknown instance format: \"" << format << "\"" << "\033[0m" << std::endl;
        assert(false);
    }
}

Instance::Instance(VertexId vertex_number):
    vertices_(vertex_number)
{
    for (VertexId v = 0; v < vertex_number; ++v)
        vertices_[v].id = v;
}

void Instance::add_edge(VertexId v1, VertexId v2, bool check_duplicate)
{
    assert(v1 < vertex_number());
    assert(v2 < vertex_number());
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
    if (degree_max_ < (VertexId)vertices_[v1].edges.size())
        degree_max_ = vertices_[v1].edges.size();

    VertexNeighbor vn2;
    vn2.e = e.id;
    vn2.v = v1;
    vertices_[v2].edges.push_back(vn2);
    if (degree_max_ < (VertexId)vertices_[v2].edges.size())
        degree_max_ = vertices_[v2].edges.size();
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
            VertexId vertex_number = stol(line[2]);
            vertices_.resize(vertex_number);
            for (VertexId v = 0; v < vertex_number; ++v)
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

