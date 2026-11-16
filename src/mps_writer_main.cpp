#include "coloringsolver/algorithms/milp.hpp"

#include "optimizationtools/graph/adjacency_list_graph.hpp"

#include <boost/program_options.hpp>

using namespace coloringsolver;

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    // Parse program options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("input,i", po::value<std::string>()->required(), "set input file (required)")
        ("format,f", po::value<std::string>(), "set input file format (default: dimacs)")
        ("remove-duplicate-edges", "remove duplicate edges from the graph")
        ("solver,", po::value<mathoptsolverscmake::SolverName>()->default_value(mathoptsolverscmake::SolverName::Highs), "set solver")
        ("output,o", po::value<std::string>()->required(), "set output MPS file (required)")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;
        return 1;
    }

    std::string instance_format = (vm.count("format"))?
        vm["format"].as<std::string>():
        "dimacs";

    // Build instance.
    optimizationtools::AdjacencyListGraphBuilder graph_builder;
    graph_builder.read(
            vm["input"].as<std::string>(),
            instance_format);
    if (vm.count("remove-duplicate-edges"))
        graph_builder.remove_duplicate_edges();
    const Instance instance(
            std::make_shared<const optimizationtools::AdjacencyListGraph>(
                graph_builder.build()));

    // Write the MPS file.
    mathoptsolverscmake::SolverName solver = vm["solver"].as<mathoptsolverscmake::SolverName>();
    write_mps(instance, solver, vm["output"].as<std::string>());

    return 0;
}
