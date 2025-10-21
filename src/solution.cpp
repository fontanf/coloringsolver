#include "coloringsolver/solution.hpp"

#include <fstream>
#include <iomanip>

using namespace coloringsolver;

Solution::Solution(const Instance& instance):
    instance_(&instance),
    map_(
            instance.graph().number_of_vertices(),
            std::max(
                instance.graph().highest_degree(),
                instance.graph().number_of_vertices())),
    number_of_conflicts_(instance.graph().number_of_vertices(), 0)
{
}

Solution::Solution(
        const Instance& instance,
        const std::string& certificate_path):
    Solution(instance)
{
    if (certificate_path.empty())
        return;
    std::ifstream file(certificate_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + certificate_path + "\".");
    }

    ColorId color_id;
    for (VertexId vertex_id = 0;
            vertex_id < instance.graph().number_of_vertices();
            ++vertex_id) {
        file >> color_id;
        set(vertex_id, color_id);
    }
}

std::ostream& Solution::format(
        std::ostream& os,
        int verbosity_level) const
{
    if (verbosity_level >= 1) {
        os
            << "Number of vertices:   " << optimizationtools::Ratio<VertexId>(number_of_vertices(), instance().graph().number_of_vertices()) << std::endl
            << "Number of conflicts:  " << number_of_conflicts() << std::endl
            << "Feasible:             " << feasible() << std::endl
            << "Number of colors:     " << number_of_colors() << std::endl
            ;
    }

    if (verbosity_level >= 2) {
        os << std::endl
            << std::setw(12) << "Vertex"
            << std::setw(12) << "Color"
            << std::endl
            << std::setw(12) << "------"
            << std::setw(12) << "-----"
            << std::endl;
        for (VertexId vertex_id = 0;
                vertex_id < instance().graph().number_of_vertices();
                ++vertex_id) {
            os
                << std::setw(12) << vertex_id
                << std::setw(12) << color(vertex_id)
                << std::endl;
        }
    }

    return os;
}

void Solution::write(const std::string& certificate_path) const 
{
    if (certificate_path.empty())
        return;
    std::ofstream file(certificate_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + certificate_path + "\".");
    }

    for (VertexId vertex_id = 0;
            vertex_id < instance().graph().number_of_vertices();
            ++vertex_id)
        file << color(vertex_id) << std::endl;
    file.close();
}

nlohmann::json Solution::to_json() const
{
    return nlohmann::json {
        {"NumberOfVertices", number_of_vertices()},
        {"NumberOfConflicts", number_of_conflicts()},
        {"Feasible", feasible()},
        {"NumberOfColors", number_of_colors()}
    };
}
