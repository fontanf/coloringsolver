load("//coloringsolver:variables.bzl", "STDCPP",
         "CPLEX_COPTS", "CPLEX_DEP", "CPOPTIMIZER_DEP")

cc_library(
        name = "algorithms",
        hdrs = [
                "greedy.hpp",
                "localsearch.hpp",
                "milp_cplex.hpp",
                "columngeneration.hpp",
                "algorithms.hpp",
        ],
        srcs = [
                "greedy.cpp",
                "localsearch.cpp",
                "milp_cplex.cpp",
                "columngeneration.cpp",
                "algorithms.cpp",
        ],
        deps = [
                "//coloringsolver:coloringsolver",
                "@columngenerationsolver//columngenerationsolver:columngenerationsolver",
                "@stablesolver//stablesolver/algorithms:algorithms",
                "@boost//:program_options",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

