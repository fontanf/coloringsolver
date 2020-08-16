load("//coloringsolver:variables.bzl", "STDCPP",
         "CPLEX_COPTS", "CPLEX_DEP", "CPOPTIMIZER_DEP")

cc_library(
        name = "algorithms",
        hdrs = [
                "greedy.hpp",
                "branchandcut_cplex.hpp",
                "algorithms.hpp",
        ],
        srcs = [
                "greedy.cpp",
                "branchandcut_cplex.cpp",
                "algorithms.cpp",
        ],
        deps = [
                "//coloringsolver:coloringsolver",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)
