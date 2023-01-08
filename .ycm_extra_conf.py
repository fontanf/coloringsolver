def Settings(**kwargs):
    return {
            'flags': [
                '-x', 'c++',
                '-Wall', '-Wextra', '-Werror',
                '-I', '.',

                '-I', './bazel-coloringsolver/external/'
                'json/single_include/',

                '-I', './bazel-coloringsolver/external/'
                'googletest/googletest/include/',

                '-I', './bazel-coloringsolver/external/'
                'boost/',

                # optimizationtools
                '-I', './bazel-coloringsolver/external/'
                # '-I', './../'
                'optimizationtools/',

                # columngenerationsolver
                '-I', './bazel-coloringsolver/external/'
                # '-I', './../'
                'columngenerationsolver/',

                # stablesolver
                '-I', './bazel-coloringsolver/external/'
                # '-I', './../'
                'stablesolver/',

                # CPLEX
                '-DCPLEX_FOUND',
                '-DIL_STD',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/concert/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cplex/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cpoptimizer/include/',
                ],
            }
