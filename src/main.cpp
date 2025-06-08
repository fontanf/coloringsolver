#include "coloringsolver/algorithms/greedy.hpp"
#include "coloringsolver/algorithms/milp_cplex.hpp"
#include "coloringsolver/algorithms/local_search_row_weighting.hpp"
#include "coloringsolver/algorithms/column_generation.hpp"

#include <boost/program_options.hpp>

using namespace coloringsolver;

namespace po = boost::program_options;

void read_args(
        Parameters& parameters,
        const po::variables_map& vm)
{
    parameters.timer.set_sigint_handler();
    parameters.messages_to_stdout = true;
    if (vm.count("time-limit"))
        parameters.timer.set_time_limit(vm["time-limit"].as<double>());
    if (vm.count("verbosity-level"))
        parameters.verbosity_level = vm["verbosity-level"].as<int>();
    if (vm.count("log"))
        parameters.log_path = vm["log"].as<std::string>();
    parameters.log_to_stderr = vm.count("log-to-stderr");

    bool only_write_at_the_end = vm.count("only-write-at-the-end");
    if (!only_write_at_the_end) {

        std::string certificate_path;
        if (vm.count("certificate"))
            certificate_path = vm["certificate"].as<std::string>();

        std::string json_output_path;
        if (vm.count("output"))
            json_output_path = vm["output"].as<std::string>();

        parameters.new_solution_callback = [
            json_output_path,
            certificate_path](
                    const Output& output,
                    const std::string&)
        {
            if (!certificate_path.empty())
                output.solution.write(certificate_path);
            if (!json_output_path.empty())
                output.write_json_output(json_output_path);
        };
    }
}

Output run(
        const Instance& instance,
        const po::variables_map& vm)
{
    std::mt19937_64 generator(0);
    if (vm.count("seed"))
        generator.seed(vm["seed"].as<Seed>());
    Solution solution = (vm.count("initial-solution"))?
            Solution(instance, vm["initial-solution"].as<std::string>()):
            Solution(instance);

    // Run algorithm.
    std::string algorithm = vm["algorithm"].as<std::string>();
    if (algorithm == "greedy") {
        GreedyParameters parameters;
        read_args(parameters, vm);
        if (vm.count("ordering"))
            parameters.ordering = vm["ordering"].as<Ordering>();
        if (vm.count("reverse"))
            parameters.reverse = vm["reverse"].as<bool>();
        return greedy(instance, parameters);
    } else if (algorithm == "greedy-dsatur"
            || algorithm == "dsatur") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_dsatur(instance, parameters);
#if CPLEX_FOUND
    } else if (algorithm == "milp-assignment-cplex") {
        MilpAssignmentCplexParameters parameters;
        read_args(parameters, vm);
        return milp_assignment_cplex(instance, parameters);
    } else if (algorithm == "milp-representatives-cplex") {
        MilpRepresentativesCplexParameters parameters;
        read_args(parameters, vm);
        return milp_representatives_cplex(instance, parameters);
    } else if (algorithm == "milp-partial-ordering-cplex") {
        MilpPartialOrderingCplexParameters parameters;
        read_args(parameters, vm);
        return milp_partialordering_cplex(instance, parameters);
    } else if (algorithm == "MilpPartialOrdering2CplexParameters") {
        MilpCplexParameters parameters;
        read_args(parameters, vm);
        return milp_partialordering2_cplex(instance, parameters);
#endif
    } else if (algorithm == "local-search-row-weighting") {
        LocalSearchRowWeightingParameters parameters;
        read_args(parameters, vm);
        if (vm.count("maximum-number-of-iterations")) {
            parameters.maximum_number_of_iterations
                = vm["maximum-number-of-iterations"].as<int>();
        }
        if (vm.count("maximum-number-of-iterations-without-improvement")) {
            parameters.maximum_number_of_iterations_without_improvement
                = vm["maximum-number-of-iterations-without-improvement"].as<int>();
        }
        return local_search_row_weighting(instance, generator, parameters);
    } else if (algorithm == "local-search-row-weighting-2") {
        LocalSearchRowWeighting2Parameters parameters;
        read_args(parameters, vm);
        if (vm.count("maximum-number-of-iterations")) {
            parameters.maximum_number_of_iterations
                = vm["maximum-number-of-iterations"].as<int>();
        }
        if (vm.count("maximum-number-of-iterations-without-improvement")) {
            parameters.maximum_number_of_iterations_without_improvement
                = vm["maximum-number-of-iterations-without-improvement"].as<int>();
        }
        return local_search_row_weighting_2(instance, generator, parameters);
    } else if (algorithm == "column-generation-greedy") {
        ColumnGenerationParameters parameters;
        read_args(parameters, vm);
        if (vm.count("linear-programming-solver")) {
            parameters.linear_programming_solver
                = vm["linear-programming-solver"].as<columngenerationsolver::SolverName>();
        }
        return column_generation_heuristic_greedy(instance, parameters);
    } else if (algorithm == "column-generation-limited-discrepancy-search") {
        ColumnGenerationParameters parameters;
        read_args(parameters, vm);
        if (vm.count("linear-programming-solver")) {
            parameters.linear_programming_solver
                = vm["linear-programming-solver"].as<columngenerationsolver::SolverName>();
        }
        return column_generation_heuristic_limited_discrepancy_search(instance, parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm + "\".");
    }
}

int main(int argc, char *argv[])
{
    // Parse program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("algorithm,a", po::value<std::string>()->required(), "set algorithm")
        ("input,i", po::value<std::string>()->required(), "set input file (required)")
        ("format,f", po::value<std::string>(), "set input file format (default: standard)")
        ("output,o", po::value<std::string>(), "set JSON output file")
        ("initial-solution,", po::value<std::string>(), "")
        ("certificate,c", po::value<std::string>(), "set certificate file")
        ("seed,s", po::value<Seed>(), "set seed")
        ("time-limit,t", po::value<double>(), "set time limit in seconds")
        ("verbosity-level,v", po::value<int>(), "set verbosity level")
        ("only-write-at-the-end,e", "only write output and certificate files at the end")
        ("log,l", po::value<std::string>(), "set log file")
        ("log-to-stderr", "write log to stderr")

        ("ordering,", po::value<Ordering>(), "set the ordering")
        ("reverse,", po::value<bool>(), "set reverse")
        ("maximum-number-of-iterations,", po::value<int>(), "set the maximum number of iterations")
        ("maximum-number-of-iterations-without-improvement,", po::value<int>(), "set the maximum number of iterations without improvement")
        ("linear-programming-solver", po::value<columngenerationsolver::SolverName>(), "set linear programming solver")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;;
        return 1;
    }
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        return 1;
    }

    // Build instance.
    const Instance instance(
            vm["input"].as<std::string>(),
            (vm.count("format")? vm["format"].as<std::string>(): "dimacs"));

    // Run.
    Output output = run(instance, vm);

    // Write outputs.
    if (vm.count("certificate"))
        output.solution.write(vm["certificate"].as<std::string>());
    if (vm.count("output"))
        output.write_json_output(vm["output"].as<std::string>());

    return 0;
}
