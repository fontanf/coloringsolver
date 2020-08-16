#include "coloringsolver/solution.hpp"

#include <iomanip>

using namespace coloringsolver;

Solution::Solution(const Instance& instance):
    instance_(instance),
    map_(instance.vertex_number(), instance.vertex_number()),
    conflicts_(instance.edge_number()),
    penalties_(instance.edge_number(), 1),
    penalty_(instance.edge_number())
{
}

Solution::Solution(const Instance& instance, std::string filepath):
    instance_(instance),
    map_(instance.vertex_number(), instance.vertex_number()),
    conflicts_(instance.edge_number()),
    penalties_(instance.edge_number(), 1),
    penalty_(instance.edge_number())
{
    if (filepath.empty())
        return;
    std::ifstream file(filepath);
    if (!file.good()) {
        std::cerr << "\033[31m" << "ERROR, unable to open file \"" << filepath << "\"" << "\033[0m" << std::endl;
        return;
    }

    ColorId color_number;
    ColorId c;
    file >> color_number;
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        file >> c;
        set(v, c);
    }
}

Solution::Solution(const Solution& solution):
    instance_(solution.instance_),
    map_(solution.map_),
    conflicts_(solution.conflicts_),
    penalties_(solution.penalties_),
    penalty_(solution.penalty_)
{ }

Solution& Solution::operator=(const Solution& solution)
{
    if (this != &solution) {
        assert(&instance_ == &solution.instance_);
        map_       = solution.map_;
        conflicts_ = solution.conflicts_;
        penalties_ = solution.penalties_;
        penalty_   = solution.penalty_;
    }
    return *this;
}

void Solution::set_penalty(EdgeId e, Penalty p)
{
    if (conflicts_.contains(e))
        penalty_ -= penalties_[e];
    penalties_[e] = p;
    if (conflicts_.contains(e))
        penalty_ += penalties_[e];
}

void Solution::increment_penalty(EdgeId e, Penalty p)
{
    penalties_[e] += p;
    if (conflicts_.contains(e))
        penalty_ += p;
}

void Solution::write(std::string filepath) const 
{
    if (filepath.empty())
        return;
    std::ofstream cert(filepath);
    if (!cert.good()) {
        std::cerr << "\033[31m" << "ERROR, unable to open file \"" << filepath << "\"" << "\033[0m" << std::endl;
        assert(false);
        return;
    }

    cert << color_number() << std::endl;
    for (VertexId v = 0; v < instance().vertex_number(); ++v)
        cert << color(v) << " ";
    cert.close();
}

std::ostream& coloringsolver::operator<<(std::ostream& os, const Solution& solution)
{
    os << solution.color_number() << std::endl;
    for (VertexId v = 0; v < solution.instance().vertex_number(); ++v)
        os << v << "|" << solution.color(v) << " ";
    return os;
}

/*********************************** Output ***********************************/

Output::Output(const Instance& instance, Info& info): solution(instance)
{
    VER(info, std::left << std::setw(12) << "T (s)");
    VER(info, std::left << std::setw(12) << "UB");
    VER(info, std::left << std::setw(12) << "LB");
    VER(info, std::left << std::setw(12) << "GAP");
    VER(info, std::left << std::setw(12) << "GAP (%)");
    VER(info, "");
    VER(info, std::endl);
    print(info, std::stringstream(""));
}

void Output::print(Info& info, const std::stringstream& s) const
{
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    double t = round(info.elapsed_time() * 10000) / 10000;

    VER(info, std::left << std::setw(12) << t);
    VER(info, std::left << std::setw(12) << upper_bound());
    VER(info, std::left << std::setw(12) << lower_bound);
    VER(info, std::left << std::setw(12) << upper_bound() - lower_bound);
    VER(info, std::left << std::setw(12) << gap);
    VER(info, s.str() << std::endl);

    if (!info.output->onlywriteattheend)
        info.write_ini();
}

void Output::update_solution(
        const Solution& solution_new,
        const std::stringstream& s,
        Info& info)
{
    info.output->mutex_sol.lock();

    if (solution_new.feasible()
            && (!solution.feasible() || solution.color_number() > solution_new.color_number())) {
        solution = solution_new;
        print(info, s);

        info.output->sol_number++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Solution" + std::to_string(info.output->sol_number);
        PUT(info, sol_str, "Value", solution.color_number());
        PUT(info, sol_str, "Time", t);
        PUT(info, sol_str, "String", s.str());
        if (!info.output->onlywriteattheend) {
            info.write_ini();
            solution.write(info.output->certfile);
        }
    }

    info.output->mutex_sol.unlock();
}

void Output::update_lower_bound(ColorId lower_bound_new, const std::stringstream& s, Info& info)
{
    if (lower_bound >= lower_bound_new)
        return;

    info.output->mutex_sol.lock();

    if (lower_bound < lower_bound_new) {
        lower_bound = lower_bound_new;
        print(info, s);

        info.output->bnd_number++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Bound" + std::to_string(info.output->bnd_number);
        PUT(info, sol_str, "Bound", lower_bound);
        PUT(info, sol_str, "Time", t);
        PUT(info, sol_str, "String", s.str());
        if (!info.output->onlywriteattheend)
            solution.write(info.output->certfile);
    }

    info.output->mutex_sol.unlock();
}

Output& Output::algorithm_end(Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    PUT(info, "Solution", "Value", upper_bound());
    PUT(info, "Bound", "Value", lower_bound);
    PUT(info, "Solution", "Time", t);
    PUT(info, "Bound", "Time", t);
    VER(info, "---" << std::endl
            << "Value: " << upper_bound() << std::endl
            << "Bound: " << lower_bound << std::endl
            << "Gap: " << upper_bound() - lower_bound << std::endl
            << "Gap (%): " << gap << std::endl
            << "Time (s): " << t << std::endl);

    info.write_ini();
    solution.write(info.output->certfile);
    return *this;
}

ColorId coloringsolver::algorithm_end(ColorId lower_bound, Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    PUT(info, "Bound", "Value", lower_bound);
    PUT(info, "Bound", "Time", t);
    VER(info, "---" << std::endl
            << "Bound: " << lower_bound << std::endl
            << "Time (s): " << t << std::endl);

    info.write_ini();
    return lower_bound;
}

