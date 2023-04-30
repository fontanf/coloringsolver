#include "coloringsolver/solution.hpp"

#include <iomanip>

using namespace coloringsolver;

Solution::Solution(const Instance& instance):
    instance_(&instance),
    map_(
            instance.graph().number_of_vertices(),
            std::max(
                instance.graph().maximum_degree(),
                instance.graph().number_of_vertices())),
    number_of_conflicts_(instance.graph().number_of_vertices(), 0)
{
}

Solution::Solution(
        const Instance& instance,
        std::string certificate_path):
    Solution(instance)
{
    if (certificate_path.empty())
        return;
    std::ifstream file(certificate_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + certificate_path + "\".");
    }

    ColorId number_of_colors;
    ColorId color_id;
    file >> number_of_colors;
    for (VertexId vertex_id = 0;
            vertex_id < instance.graph().number_of_vertices();
            ++vertex_id) {
        file >> color_id;
        set(vertex_id, color_id);
    }
}

std::ostream& Solution::print(
        std::ostream& os,
        int verbose) const
{
    if (verbose >= 1) {
        os
            << "Number of vertices:              " << optimizationtools::Ratio<VertexId>(number_of_vertices(), instance().graph().number_of_vertices()) << std::endl
            << "Number of conflicts:             " << number_of_conflicts() << std::endl
            << "Feasible:                        " << feasible() << std::endl
            << "Number of colors:                " << number_of_colors() << std::endl
            ;
    }

    if (verbose >= 2) {
        os << std::endl
            << std::setw(12) << "VertexId"
            << std::setw(12) << "Color"
            << std::endl
            << std::setw(12) << "--------"
            << std::setw(12) << "------"
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

void Solution::write(std::string certificate_path) const 
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

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Output ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Output::Output(
        const Instance& instance,
        optimizationtools::Info& info):
    solution(instance)
{
    info.os()
        << std::setw(12) << "T (s)"
        << std::setw(12) << "UB"
        << std::setw(12) << "LB"
        << std::setw(12) << "GAP"
        << std::setw(12) << "GAP (%)"
        << std::setw(24) << "Comment"
        << std::endl
        << std::setw(12) << "-----"
        << std::setw(12) << "--"
        << std::setw(12) << "--"
        << std::setw(12) << "---"
        << std::setw(12) << "-------"
        << std::setw(24) << "-------"
        << std::endl;
    print(info, std::stringstream(""));
}

void Output::print(optimizationtools::Info& info, const std::stringstream& s) const
{
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    double t = info.elapsed_time();
    std::streamsize precision = std::cout.precision();

    info.os()
        << std::setw(12) << std::fixed << std::setprecision(3) << t << std::defaultfloat << std::setprecision(precision)
        << std::setw(12) << upper_bound()
        << std::setw(12) << lower_bound
        << std::setw(12) << upper_bound() - lower_bound
        << std::setw(12) << std::fixed << std::setprecision(2) << gap << std::defaultfloat << std::setprecision(precision)
        << std::setw(24) << s.str()
        << std::endl;

    if (!info.output->only_write_at_the_end)
        info.write_json_output();
}

void Output::update_solution(
        const Solution& solution_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    info.lock();

    if (solution_new.feasible()
            && (!solution.feasible() || solution.number_of_colors() > solution_new.number_of_colors())) {
        // Update solution
        if (solution.number_of_vertices() == 0) {
            solution = solution_new;
        } else {
            for (VertexId vertex_id = 0;
                    vertex_id < solution.instance().graph().number_of_vertices();
                    ++vertex_id) {
                if (solution.color(vertex_id) != solution_new.color(vertex_id))
                    solution.set(vertex_id, solution_new.color(vertex_id));
            }
        }
        print(info, s);

        info.output->number_of_solutions++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Solution" + std::to_string(info.output->number_of_solutions);
        info.add_to_json(sol_str, "Value", solution.number_of_colors());
        info.add_to_json(sol_str, "Time", t);
        info.add_to_json(sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end) {
            info.write_json_output();
            solution.write(info.output->certificate_path);
        }
    }

    info.unlock();
}

void Output::update_lower_bound(
        ColorId lower_bound_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    if (lower_bound >= lower_bound_new)
        return;

    info.lock();

    if (lower_bound < lower_bound_new) {
        lower_bound = lower_bound_new;
        print(info, s);

        info.output->number_of_bounds++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Bound" + std::to_string(info.output->number_of_bounds);
        info.add_to_json(sol_str, "Bound", lower_bound);
        info.add_to_json(sol_str, "Time", t);
        info.add_to_json(sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end)
            info.write_json_output();
    }

    info.unlock();
}

Output& Output::algorithm_end(optimizationtools::Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    info.add_to_json("Solution", "Value", upper_bound());
    info.add_to_json("Bound", "Value", lower_bound);
    info.add_to_json("Solution", "Time", t);
    info.add_to_json("Bound", "Time", t);
    info.os()
        << std::endl
        << "Final statistics" << std::endl
        << "----------------" << std::endl
        << "Value:                 " << upper_bound() << std::endl
        << "Bound:                 " << lower_bound << std::endl
        << "Gap:                   " << upper_bound() - lower_bound << std::endl
        << "Gap (%):               " << gap << std::endl
        << "Time (s):              " << t << std::endl;
    print_statistics(info);
    info.os() << std::endl
        << "Solution" << std::endl
        << "--------" << std::endl ;
    solution.print(info.os(), info.verbosity_level());

    info.write_json_output();
    solution.write(info.output->certificate_path);
    return *this;
}

