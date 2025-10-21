#include "tests.hpp"
#include "coloringsolver/algorithms/milp.hpp"

using namespace coloringsolver;

TEST_P(ExactAlgorithmTest, ExactAlgorithm)
{
    TestParams test_params = GetParam();
    const Instance instance = get_instance(test_params.files);
    const Solution solution = get_solution(instance, test_params.files);
    auto output = test_params.algorithm(instance);
    std::cout << std::endl;
    std::cout << "Expected solution" << std::endl;
    std::cout << "-----------------" << std::endl;
    solution.format(std::cout, 1);
    EXPECT_EQ(output.solution.objective_value(), solution.objective_value());
    EXPECT_EQ(output.bound, solution.objective_value());
}

INSTANTIATE_TEST_SUITE_P(
        ColoringMilpAssignment,
        ExactAlgorithmTest,
        testing::ValuesIn(get_test_params(
                [](const Instance& instance)
                {
                    MilpAssignmentParameters milp_parameters;
                    milp_parameters.break_symmetries = false;
                    return milp_assignment(instance, milp_parameters);
                }, {
                    get_test_instance_paths(get_path({"data", "test_milp_assignment.txt"})),
                })));

INSTANTIATE_TEST_SUITE_P(
        ColoringMilpAssignmentBreakSymmetries,
        ExactAlgorithmTest,
        testing::ValuesIn(get_test_params(
                [](const Instance& instance)
                {
                    MilpAssignmentParameters milp_parameters;
                    milp_parameters.break_symmetries = true;
                    return milp_assignment(instance, milp_parameters);
                }, {
                    get_test_instance_paths(get_path({"data", "test_milp_assignment_break_symmetries.txt"})),
                })));

INSTANTIATE_TEST_SUITE_P(
        ColoringMilpRepresentatives,
        ExactAlgorithmTest,
        testing::ValuesIn(get_test_params(
                [](const Instance& instance)
                {
                    MilpParameters milp_parameters;
                    return milp_representatives(instance, milp_parameters);
                }, {
                    get_test_instance_paths(get_path({"data", "test_milp_representatives.txt"})),
                })));

INSTANTIATE_TEST_SUITE_P(
        ColoringMilpPartialOrdering,
        ExactAlgorithmTest,
        testing::ValuesIn(get_test_params(
                [](const Instance& instance)
                {
                    MilpPartialOrderingParameters milp_parameters;
                    milp_parameters.hybrid = false;
                    return milp_partial_ordering(instance, milp_parameters);
                }, {
                    get_test_instance_paths(get_path({"data", "test_milp_partial_ordering.txt"})),
                })));

INSTANTIATE_TEST_SUITE_P(
        ColoringMilpPartialOrderingHybrid,
        ExactAlgorithmTest,
        testing::ValuesIn(get_test_params(
                [](const Instance& instance)
                {
                    MilpPartialOrderingParameters milp_parameters;
                    milp_parameters.hybrid = true;
                    return milp_partial_ordering(instance, milp_parameters);
                }, {
                    get_test_instance_paths(get_path({"data", "test_milp_partial_ordering_hybrid.txt"})),
                })));
