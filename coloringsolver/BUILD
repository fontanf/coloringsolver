config_setting(name = "cplex_build",       values = {"define": "cplex=true"},       visibility = ["//visibility:public"])

load("//coloringsolver:variables.bzl", "STDCPP")

cc_library(
        name = "coloringsolver",
        hdrs = [
                "instance.hpp",
                "solution.hpp",
        ],
        srcs = [
                "instance.cpp",
                "solution.cpp",
        ],
        deps = [
                "@optimizationtools//optimizationtools:info",
                "@optimizationtools//optimizationtools:utils",
                "@optimizationtools//optimizationtools:indexed_collections",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "main",
        srcs = ["main.cpp"],
        deps = [
                "//coloringsolver/algorithms:algorithms",
        ],
        linkopts = select({
                    "@bazel_tools//src/conditions:windows": [],
                    "//conditions:default":                 ["-lpthread"],
        }),
)

