def Settings( **kwargs ):
    return {
            'flags': [
                '-x', 'c++',
                '-Wall', '-Wextra', '-Werror',
                '-DIL_STD', # Cplex
                '-I', '.',
                '-I', './bazel-coloringsolver/external/json/single_include',
                '-I', './bazel-coloringsolver/external/googletest/googletest/include',
                # '-I', './bazel-coloringsolver/external/optimizationtools',
                '-I', './../optimizationtools',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/concert/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cplex/include/',
                '-I', '/opt/ibm/ILOG/CPLEX_Studio129/cpoptimizer/include/',
                ],
            }

