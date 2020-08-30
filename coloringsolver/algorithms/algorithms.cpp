#include "coloringsolver/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace coloringsolver;
namespace po = boost::program_options;

BranchAndCutAssignmentCplexOptionalParameters read_branchandcut_assignment_cplex_args(const std::vector<char*>& argv)
{
    BranchAndCutAssignmentCplexOptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("break-symmetries,s", po::value<bool>(&parameters.break_symmetries), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (po::required_option e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

LocalSearchOptionalParameters read_localsearch_args(const std::vector<char*>& argv)
{
    LocalSearchOptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("threads,t", po::value<Counter>(&parameters.thread_number), "")
        ("iterations,i", po::value<Counter>(&parameters.iteration_limit), "")
        ("iterations-without-improvment,w", po::value<Counter>(&parameters.iteration_without_improvment_limit), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (po::required_option e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

Output coloringsolver::run(
        std::string algorithm, const Instance& instance, std::mt19937_64& generator, Info info)
{
    std::vector<std::string> algorithm_args = po::split_unix(algorithm);
    std::vector<char*> algorithm_argv;
    for (Counter i = 0; i < (Counter)algorithm_args.size(); ++i)
        algorithm_argv.push_back(const_cast<char*>(algorithm_args[i].c_str()));

    if (algorithm.empty() || algorithm_args[0].empty()) {
        std::cerr << "\033[32m" << "ERROR, missing algorithm." << "\033[0m" << std::endl;
        return Output(instance, info);

    } else if (algorithm_args[0] == "greedy_largestfirst") {
        return greedy_largestfirst(instance, info);
    } else if (algorithm_args[0] == "greedy_incidencedegree") {
        return greedy_incidencedegree(instance, info);
    } else if (algorithm_args[0] == "greedy_smallestlast") {
        return greedy_smallestlast(instance, info);
    } else if (algorithm_args[0] == "greedy_dynamiclargestfirst") {
        return greedy_dynamiclargestfirst(instance, info);
    } else if (algorithm_args[0] == "greedy_dsatur") {
        return greedy_dsatur(instance, info);
#if CPLEX_FOUND
    } else if (algorithm_args[0] == "branchandcut_assignment_cplex") {
        auto parameters = read_branchandcut_assignment_cplex_args(algorithm_argv);
        parameters.info = info;
        return branchandcut_assignment_cplex(instance, parameters);
    } else if (algorithm_args[0] == "branchandcut_representatives_cplex") {
        BranchAndCutRepresentativesCplexOptionalParameters parameters;
        parameters.info = info;
        return branchandcut_representatives_cplex(instance, parameters);
    } else if (algorithm_args[0] == "branchandcut_partialordering_cplex") {
        BranchAndCutPartialOrderingCplexOptionalParameters parameters;
        parameters.info = info;
        return branchandcut_partialordering_cplex(instance, parameters);
    } else if (algorithm_args[0] == "branchandcut_partialordering2_cplex") {
        BranchAndCutPartialOrdering2CplexOptionalParameters parameters;
        parameters.info = info;
        return branchandcut_partialordering2_cplex(instance, parameters);
#endif
    } else if (algorithm_args[0] == "localsearch") {
        auto parameters = read_localsearch_args(algorithm_argv);
        parameters.info = info;
        return localsearch(instance, generator, parameters);

    } else {
        std::cerr << "\033[31m" << "ERROR, unknown algorithm: " << algorithm_argv[0] << "\033[0m" << std::endl;
        assert(false);
        return Output(instance, info);
    }
}

