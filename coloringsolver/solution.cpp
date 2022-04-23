#include "coloringsolver/solution.hpp"

#include <iomanip>

using namespace coloringsolver;

Solution::Solution(const Instance& instance):
    instance_(instance),
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
    instance_(instance),
    map_(
            instance.graph().number_of_vertices(),
            std::max(
                instance.graph().maximum_degree(),
                instance.graph().number_of_vertices())),
    number_of_conflicts_(instance.graph().number_of_vertices(), 0)
{
    if (certificate_path.empty())
        return;
    std::ifstream file(certificate_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + certificate_path + "\".");
    }

    ColorId number_of_colors;
    ColorId c;
    file >> number_of_colors;
    for (VertexId v = 0; v < instance.graph().number_of_vertices(); ++v) {
        file >> c;
        set(v, c);
    }
}

Solution::Solution(const Solution& solution):
    instance_(solution.instance_),
    map_(solution.map_),
    conflicts_(solution.conflicts_),
    number_of_conflicts_(solution.number_of_conflicts_),
    total_number_of_conflicts_(solution.total_number_of_conflicts_)
{ }

Solution& Solution::operator=(const Solution& solution)
{
    if (this != &solution) {
        if (&instance_ != &solution.instance_) {
            throw std::runtime_error(
                    "Assign a solution to a solution from a different instance.");
        }

        map_ = solution.map_;
        conflicts_ = solution.conflicts_;
        number_of_conflicts_ = solution.number_of_conflicts_;
        total_number_of_conflicts_ = solution.total_number_of_conflicts_;
    }
    return *this;
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

    for (VertexId v = 0; v < instance().graph().number_of_vertices(); ++v)
        file << color(v) << std::endl;
    file.close();
}

std::ostream& coloringsolver::operator<<(
        std::ostream& os,
        const Solution& solution)
{
    os << solution.number_of_colors() << std::endl;
    for (VertexId v = 0; v < solution.instance().graph().number_of_vertices(); ++v)
        os << v << "|" << solution.color(v) << " ";
    return os;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Output ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Output::Output(
        const Instance& instance,
        optimizationtools::Info& info):
    solution(instance)
{
    FFOT_VER(info,
               std::setw(12) << "T (s)"
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
            << std::endl);
    print(info, std::stringstream(""));
}

void Output::print(optimizationtools::Info& info, const std::stringstream& s) const
{
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    double t = info.elapsed_time();
    std::streamsize precision = std::cout.precision();

    FFOT_VER(info,
               std::setw(12) << std::fixed << std::setprecision(3) << t << std::defaultfloat << std::setprecision(precision)
            << std::setw(12) << upper_bound()
            << std::setw(12) << lower_bound
            << std::setw(12) << upper_bound() - lower_bound
            << std::setw(12) << std::fixed << std::setprecision(2) << gap << std::defaultfloat << std::setprecision(precision)
            << std::setw(24) << s.str()
            << std::endl);

    if (!info.output->only_write_at_the_end)
        info.write_json_output();
}

void Output::update_solution(
        const Solution& solution_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    info.output->mutex_solutions.lock();

    if (solution_new.feasible()
            && (!solution.feasible() || solution.number_of_colors() > solution_new.number_of_colors())) {
        // Update solution
        if (solution.number_of_vertices() == 0) {
            solution = solution_new;
        } else {
            for (VertexId v = 0; v < solution.instance().graph().number_of_vertices(); ++v)
                if (solution.color(v) != solution_new.color(v))
                    solution.set(v, solution_new.color(v));
        }
        print(info, s);

        info.output->number_of_solutions++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Solution" + std::to_string(info.output->number_of_solutions);
        FFOT_PUT(info, sol_str, "Value", solution.number_of_colors());
        FFOT_PUT(info, sol_str, "Time", t);
        FFOT_PUT(info, sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end) {
            info.write_json_output();
            solution.write(info.output->certificate_path);
        }
    }

    info.output->mutex_solutions.unlock();
}

void Output::update_lower_bound(
        ColorId lower_bound_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    if (lower_bound >= lower_bound_new)
        return;

    info.output->mutex_solutions.lock();

    if (lower_bound < lower_bound_new) {
        lower_bound = lower_bound_new;
        print(info, s);

        info.output->number_of_bounds++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Bound" + std::to_string(info.output->number_of_bounds);
        FFOT_PUT(info, sol_str, "Bound", lower_bound);
        FFOT_PUT(info, sol_str, "Time", t);
        FFOT_PUT(info, sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end)
            info.write_json_output();
    }

    info.output->mutex_solutions.unlock();
}

Output& Output::algorithm_end(optimizationtools::Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    FFOT_PUT(info, "Solution", "Value", upper_bound());
    FFOT_PUT(info, "Bound", "Value", lower_bound);
    FFOT_PUT(info, "Solution", "Time", t);
    FFOT_PUT(info, "Bound", "Time", t);
    FFOT_VER(info,
            std::endl
            << "Final statistics" << std::endl
            << "----------------" << std::endl
            << "Value:                 " << upper_bound() << std::endl
            << "Bound:                 " << lower_bound << std::endl
            << "Gap:                   " << upper_bound() - lower_bound << std::endl
            << "Gap (%):               " << gap << std::endl
            << "Time (s):              " << t << std::endl);

    info.write_json_output();
    solution.write(info.output->certificate_path);
    return *this;
}

ColorId coloringsolver::algorithm_end(
        ColorId lower_bound,
        optimizationtools::Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    FFOT_PUT(info, "Bound", "Value", lower_bound);
    FFOT_PUT(info, "Bound", "Time", t);
    FFOT_VER(info,
            std::endl
            << "Final statistics" << std::endl
            << "----------------" << std::endl
            << "Bound:                 " << lower_bound << std::endl
            << "Time (s):              " << t << std::endl);

    info.write_json_output();
    return lower_bound;
}

