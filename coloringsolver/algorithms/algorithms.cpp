#include "coloringsolver/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace coloringsolver;
namespace po = boost::program_options;

GreedyParameters read_greedy_args(const std::vector<char*>& argv)
{
    GreedyParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("ordering,o", po::value<Ordering>(&parameters.ordering), "")
        ("reverse,r", "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    parameters.reverse = vm.count("reverse");
    return parameters;
}

LocalSearchRowWeightingParameters read_local_search_row_weighting_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeightingParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("iterations,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iterations-without-improvement,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
        ("core,c", po::value<bool>(&parameters.enable_core_reduction), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

LocalSearchRowWeighting2Parameters read_local_search_row_weighting_2_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeighting2Parameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("iterations,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iterations-without-improvement,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
        ("core,c", po::value<bool>(&parameters.enable_core_reduction), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

ColumnGenerationParameters read_column_generation_args(const std::vector<char*>& argv)
{
    ColumnGenerationParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("linear-programming-solver,s", po::value<std::string>(&parameters.linear_programming_solver), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

#if CPLEX_FOUND
MilpAssignmentCplexParameters read_milp_assignment_cplex_args(const std::vector<char*>& argv)
{
    MilpAssignmentCplexParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("break-symmetries,s", po::value<bool>(&parameters.break_symmetries), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}
#endif

Output coloringsolver::run(
        std::string algorithm,
        const Instance& instance,
        const Solution& initial_solution,
        ColorId goal,
        std::mt19937_64& generator,
        optimizationtools::Info info)
{
    (void)initial_solution;
    std::vector<std::string> algorithm_args = po::split_unix(algorithm);
    std::vector<char*> algorithm_argv;
    for (Counter i = 0; i < (Counter)algorithm_args.size(); ++i)
        algorithm_argv.push_back(const_cast<char*>(algorithm_args[i].c_str()));

    if (algorithm.empty() || algorithm_args[0].empty()) {
        throw std::invalid_argument("Missing algorithm.");

    } else if (algorithm_args[0] == "greedy") {
        auto parameters = read_greedy_args(algorithm_argv);
        parameters.info = info;
        return greedy(instance, parameters);
    } else if (algorithm_args[0] == "greedy-dsatur") {
        return greedy_dsatur(instance, info);
#if CPLEX_FOUND
    } else if (algorithm_args[0] == "milp-assignment-cplex") {
        auto parameters = read_milp_assignment_cplex_args(algorithm_argv);
        parameters.info = info;
        return milp_assignment_cplex(instance, parameters);
    } else if (algorithm_args[0] == "milp-representatives-cplex") {
        MilpRepresentativesCplexParameters parameters;
        parameters.info = info;
        return milp_representatives_cplex(instance, parameters);
    } else if (algorithm_args[0] == "milp-partial-ordering-cplex") {
        MilpPartialOrderingCplexParameters parameters;
        parameters.info = info;
        return milp_partialordering_cplex(instance, parameters);
    } else if (algorithm_args[0] == "milp-partial-ordering-2-cplex") {
        MilpPartialOrdering2CplexParameters parameters;
        parameters.info = info;
        return milp_partialordering2_cplex(instance, parameters);
#endif
    } else if (algorithm_args[0] == "local-search-row-weighting") {
        auto parameters = read_local_search_row_weighting_args(algorithm_argv);
        parameters.info = info;
        parameters.goal = goal;
        return local_search_row_weighting(instance, generator, parameters);
    } else if (algorithm_args[0] == "local-search-row-weighting-2") {
        auto parameters = read_local_search_row_weighting_2_args(algorithm_argv);
        parameters.info = info;
        parameters.goal = goal;
        return local_search_row_weighting_2(instance, generator, parameters);
    } else if (algorithm_args[0] == "column-generation-heuristic-greedy") {
        auto parameters = read_column_generation_args(algorithm_argv);
        parameters.info = info;
        return column_generation_heuristic_greedy(instance, parameters);
    } else if (algorithm_args[0] == "column-generation-heuristic-limited-discrepancy-search") {
        auto parameters = read_column_generation_args(algorithm_argv);
        parameters.info = info;
        return column_generation_heuristic_limited_discrepancy_search(instance, parameters);
    } else if (algorithm_args[0] == "column-generation-heuristic-heuristic-tree-search") {
        auto parameters = read_column_generation_args(algorithm_argv);
        parameters.info = info;
        return column_generation_heuristic_heuristic_tree_search(instance, parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm_args[0] + "\".");
    }
}

