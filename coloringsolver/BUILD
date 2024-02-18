config_setting(name = "cplex_build",       values = {"define": "cplex=true"},       visibility = ["//visibility:public"])

load("//coloringsolver:variables.bzl", "STDCPP")

cc_library(
        name = "coloringsolver",
        hdrs = [
                "instance.hpp",
                "solution.hpp",
                "algorithm_formatter.hpp",
        ],
        srcs = [
                "instance.cpp",
                "solution.cpp",
                "algorithm_formatter.cpp",
        ],
        deps = [
                "@optimizationtools//optimizationtools/utils:utils",
                "@optimizationtools//optimizationtools/containers:containers",
                "@optimizationtools//optimizationtools/graph:graph",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "main",
        srcs = ["main.cpp"],
        deps = [
                "//coloringsolver/algorithms:greedy",
                "//coloringsolver/algorithms:local_search_row_weighting",
                "//coloringsolver/algorithms:milp_cplex",
                "//coloringsolver/algorithms:column_generation",
                "@boost//:program_options",
        ],
        linkopts = select({
                    "@bazel_tools//src/conditions:windows": [],
                    "//conditions:default":                 ["-lpthread"],
        }),
)
