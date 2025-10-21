add_library(ColoringSolver_greedy)
target_sources(ColoringSolver_greedy PRIVATE
    greedy.cpp)
target_include_directories(ColoringSolver_greedy PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ColoringSolver_greedy PUBLIC
    ColoringSolver_coloring)
add_library(ColoringSolver::greedy ALIAS ColoringSolver_greedy)
set_target_properties(ColoringSolver_greedy PROPERTIES OUTPUT_NAME "coloringsolver_greedy")
install(TARGETS ColoringSolver_greedy)

add_library(ColoringSolver_milp)
target_sources(ColoringSolver_milp PRIVATE
    milp.cpp)
target_include_directories(ColoringSolver_milp PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ColoringSolver_milp PUBLIC
    ColoringSolver_coloring
    MathOptSolversCMake::milp)
add_library(ColoringSolver::milp ALIAS ColoringSolver_milp)

add_library(ColoringSolver_local_search_row_weighting)
target_sources(ColoringSolver_local_search_row_weighting PRIVATE
    local_search_row_weighting.cpp)
target_include_directories(ColoringSolver_local_search_row_weighting PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ColoringSolver_local_search_row_weighting PUBLIC
    ColoringSolver_coloring
    ColoringSolver_greedy)
add_library(ColoringSolver::local_search_row_weighting ALIAS ColoringSolver_local_search_row_weighting)
set_target_properties(ColoringSolver_local_search_row_weighting PROPERTIES OUTPUT_NAME "coloringsolver_local_search_row_weighting")
install(TARGETS ColoringSolver_local_search_row_weighting)

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
set_target_properties(ColoringSolver_column_generation PROPERTIES OUTPUT_NAME "coloringsolver_column_generation")
install(TARGETS ColoringSolver_column_generation)
