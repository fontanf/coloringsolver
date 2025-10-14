add_library(ColoringSolver_coloring)
target_sources(ColoringSolver_coloring PRIVATE
    instance.cpp
    solution.cpp
    algorithm_formatter.cpp)
target_include_directories(ColoringSolver_coloring PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ColoringSolver_coloring PUBLIC
    OptimizationTools::utils
    OptimizationTools::containers
    OptimizationTools::graph)
add_library(ColoringSolver::coloring ALIAS ColoringSolver_coloring)
set_target_properties(ColoringSolver_coloring PROPERTIES OUTPUT_NAME "coloringsolver_coloring")
install(TARGETS ColoringSolver_coloring)

add_subdirectory(algorithms)

if(COLORINGSOLVER_BUILD_MAIN)
    add_executable(ColoringSolver_main)
    target_sources(ColoringSolver_main PRIVATE
        main.cpp)
    target_link_libraries(ColoringSolver_main PUBLIC
        ColoringSolver_greedy
        ColoringSolver_local_search_row_weighting
        ColoringSolver_column_generation
        Boost::program_options)
    if(COLORINGSOLVER_USE_CPLEX)
        target_compile_definitions(ColoringSolver_main PUBLIC
            CPLEX_FOUND=1)
        target_link_libraries(ColoringSolver_main PUBLIC
            ColoringSolver_milp_cplex)
    endif()
    set_target_properties(ColoringSolver_main PROPERTIES OUTPUT_NAME "coloringsolver")
    install(TARGETS ColoringSolver_main)
endif()
