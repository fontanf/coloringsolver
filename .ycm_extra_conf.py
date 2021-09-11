def Settings(**kwargs):
    return {
            'flags': [
                '-x', 'c++',
                '-Wall', '-Wextra', '-Werror',
                '-DIL_STD',  # Cplex
                '-I', '.',
                '-I', './bazel-coloringsolver/external/json/single_include',
                '-I', './bazel-coloringsolver/external/googletest/googletest/include',
                '-I', './bazel-coloringsolver/external/boost/',
                '-I', './bazel-coloringsolver/external/optimizationtools',
                '-I', './bazel-coloringsolver/external/columngenerationsolver',
                # '-I', './../columngenerationsolver/',
                '-I', './bazel-coloringsolver/external/stablesolver/',
                # '-I', './../stablesolver/',
                '-I', './bazel-coloringsolver/external/localsearchsolver/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/concert/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cplex/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cpoptimizer/include/',
                ],
            }
