#include "coloringsolver/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace coloringsolver;
namespace po = boost::program_options;

GreedyOptionalParameters read_greedy_args(const std::vector<char*>& argv)
{
    GreedyOptionalParameters parameters;
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

LocalSearchOptionalParameters read_localsearch_args(const std::vector<char*>& argv)
{
    LocalSearchOptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("threads,t", po::value<Counter>(&parameters.number_of_threads), "")
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

LocalSearchRowWeightingOptionalParameters read_localsearch_rowweighting_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeightingOptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("iterations,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iterations-without-improvement-limit,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
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

LocalSearchRowWeighting2OptionalParameters read_localsearch_rowweighting_2_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeighting2OptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("iterations,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iterations-without-improvement-limit,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
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

ColumnGenerationOptionalParameters read_columngeneration_args(const std::vector<char*>& argv)
{
    ColumnGenerationOptionalParameters parameters;
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
MilpAssignmentCplexOptionalParameters read_milp_assignment_cplex_args(const std::vector<char*>& argv)
{
    MilpAssignmentCplexOptionalParameters parameters;
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
        std::mt19937_64& generator,
        optimizationtools::Info info)
{
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
    } else if (algorithm_args[0] == "greedy_dsatur") {
        return greedy_dsatur(instance, info);
#if CPLEX_FOUND
    } else if (algorithm_args[0] == "milp_assignment_cplex") {
        auto parameters = read_milp_assignment_cplex_args(algorithm_argv);
        parameters.info = info;
        return milp_assignment_cplex(instance, parameters);
    } else if (algorithm_args[0] == "milp_representatives_cplex") {
        MilpRepresentativesCplexOptionalParameters parameters;
        parameters.info = info;
        return milp_representatives_cplex(instance, parameters);
    } else if (algorithm_args[0] == "milp_partialordering_cplex") {
        MilpPartialOrderingCplexOptionalParameters parameters;
        parameters.info = info;
        return milp_partialordering_cplex(instance, parameters);
    } else if (algorithm_args[0] == "milp_partialordering2_cplex") {
        MilpPartialOrdering2CplexOptionalParameters parameters;
        parameters.info = info;
        return milp_partialordering2_cplex(instance, parameters);
#endif
    } else if (algorithm_args[0] == "localsearch") {
        auto parameters = read_localsearch_args(algorithm_argv);
        parameters.info = info;
        parameters.initial_solution = &initial_solution;
        return localsearch(instance, generator, parameters);
    } else if (algorithm_args[0] == "localsearch_rowweighting") {
        auto parameters = read_localsearch_rowweighting_args(algorithm_argv);
        parameters.info = info;
        return localsearch_rowweighting(instance, generator, parameters);
    } else if (algorithm_args[0] == "localsearch_rowweighting_2") {
        auto parameters = read_localsearch_rowweighting_2_args(algorithm_argv);
        parameters.info = info;
        return localsearch_rowweighting_2(instance, generator, parameters);
    } else if (algorithm_args[0] == "columngenerationheuristic_greedy") {
        auto parameters = read_columngeneration_args(algorithm_argv);
        parameters.info = info;
        return columngenerationheuristic_greedy(instance, parameters);
    } else if (algorithm_args[0] == "columngenerationheuristic_limiteddiscrepancysearch") {
        auto parameters = read_columngeneration_args(algorithm_argv);
        parameters.info = info;
        return columngenerationheuristic_limiteddiscrepancysearch(instance, parameters);
    } else if (algorithm_args[0] == "columngenerationheuristic_heuristictreesearch") {
        auto parameters = read_columngeneration_args(algorithm_argv);
        parameters.info = info;
        return columngenerationheuristic_heuristictreesearch(instance, parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm_args[0] + "\".");
    }
}

