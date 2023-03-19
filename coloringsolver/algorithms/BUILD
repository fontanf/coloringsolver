load("//coloringsolver:variables.bzl", "STDCPP",
         "CPLEX_COPTS", "CPLEX_DEP", "CPOPTIMIZER_DEP")

cc_library(
        name = "greedy",
        hdrs = ["greedy.hpp"],
        srcs = ["greedy.cpp"],
        deps = ["//coloringsolver:coloringsolver"],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "localsearch_rowweighting",
        hdrs = ["localsearch_rowweighting.hpp"],
        srcs = ["localsearch_rowweighting.cpp"],
        deps = [
                ":greedy",
                "//coloringsolver:coloringsolver",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "milp_cplex",
        hdrs = ["milp_cplex.hpp"],
        srcs = ["milp_cplex.cpp"],
        deps = [
                "//coloringsolver:coloringsolver",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "columngeneration",
        hdrs = ["columngeneration.hpp"],
        srcs = ["columngeneration.cpp"],
        deps = [
                "//coloringsolver:coloringsolver",
                "@columngenerationsolver//columngenerationsolver:columngenerationsolver",
                "@stablesolver//stablesolver/algorithms:localsearch",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "algorithms",
        hdrs = ["algorithms.hpp"],
        srcs = ["algorithms.cpp"],
        deps = [
                ":greedy",
                ":localsearch_rowweighting",
                ":milp_cplex",
                ":columngeneration",
                "@boost//:program_options",
        ] + CPLEX_DEP,
        copts = CPLEX_COPTS,
        visibility = ["//visibility:public"],
)

