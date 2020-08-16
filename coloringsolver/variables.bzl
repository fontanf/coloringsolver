STDCPP = select({
            "@bazel_tools//src/conditions:windows": ['/std:c++latest'],
            "//conditions:default":                 ["-std=c++14"],})

CPLEX_COPTS = select({
            "//coloringsolver:cplex_build": [
                    "-DCPLEX_FOUND",
                    "-m64",
                    "-DIL_STD"],
            "//conditions:default": []})

CPLEX_DEP = select({
            "//coloringsolver:cplex_build": ["@cplex//:cplex"],
            "//conditions:default": []})
CPOPTIMIZER_DEP = select({
            "//coloringsolver:cplex_build": ["@cplex//:cpoptimizer"],
            "//conditions:default": []})

