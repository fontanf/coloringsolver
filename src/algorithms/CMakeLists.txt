add_library(ColoringSolver_greedy)
target_sources(ColoringSolver_greedy PRIVATE
    greedy.cpp)
target_include_directories(ColoringSolver_greedy PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ColoringSolver_greedy PUBLIC
    ColoringSolver_coloring)
add_library(ColoringSolver::greedy ALIAS ColoringSolver_greedy)

if(COLORINGSOLVER_USE_CPLEX)
    add_library(ColoringSolver_milp_cplex)
    target_sources(ColoringSolver_milp_cplex PRIVATE
        milp_cplex.cpp)
    target_include_directories(ColoringSolver_milp_cplex PUBLIC
        ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(ColoringSolver_milp_cplex PUBLIC
        ColoringSolver_coloring
        MathOptSolversCMake::cplex)
    add_library(ColoringSolver::milp_cplex ALIAS ColoringSolver_milp_cplex)
endif()

add_library(ColoringSolver_local_search_row_weighting)
target_sources(ColoringSolver_local_search_row_weighting PRIVATE
    local_search_row_weighting.cpp)
target_include_directories(ColoringSolver_local_search_row_weighting PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ColoringSolver_local_search_row_weighting PUBLIC
    ColoringSolver_coloring)
add_library(ColoringSolver::local_search_row_weighting ALIAS ColoringSolver_local_search_row_weighting)

add_library(ColoringSolver_column_generation)
target_sources(ColoringSolver_column_generation PRIVATE
    column_generation.cpp)
target_include_directories(ColoringSolver_column_generation PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ColoringSolver_column_generation PUBLIC
    ColoringSolver_coloring
    ColumnGenerationSolver::columngenerationsolver
    StableSolver::stable::local_search)
add_library(ColoringSolver::column_generation ALIAS ColoringSolver_column_generation)
