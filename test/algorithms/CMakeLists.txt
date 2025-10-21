add_library(ColoringSolver_tests)
target_sources(ColoringSolver_tests PRIVATE
    tests.cpp)
target_link_libraries(ColoringSolver_tests PUBLIC
    ColoringSolver_coloring
    Boost::filesystem
    GTest::gtest_main)
add_library(ColoringSolver::tests ALIAS ColoringSolver_tests)

add_executable(ColoringSolver_milp_test)
target_sources(ColoringSolver_milp_test PRIVATE
    milp_test.cpp)
target_link_libraries(ColoringSolver_milp_test
    ColoringSolver_tests
    ColoringSolver_milp
    GTest::gtest_main)
gtest_discover_tests(ColoringSolver_milp_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
