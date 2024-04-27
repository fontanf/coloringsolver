# Enable FetchContent.
include(FetchContent)

# Fetch boost.
set(BOOST_INCLUDE_LIBRARIES thread filesystem system program_options dynamic_bitset)
set(BOOST_ENABLE_CMAKE ON)
include(FetchContent)
FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.84.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Boost)

# Fetch fontanf/optimizationtools.
FetchContent_Declare(
    optimizationtools
    GIT_REPOSITORY https://github.com/fontanf/optimizationtools.git
    GIT_TAG 33a3966ece149d390ec7ce08699669b5267e64aa)
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../optimizationtools/")
FetchContent_MakeAvailable(optimizationtools)

# Fetch fontanf/mathoptsolverscmake.
if(COLORINGSOLVER_USE_CBC)
    set(MATHOPTSOLVERSCMAKE_USE_CBC ON)
endif()
if(COLORINGSOLVER_USE_CPLEX)
    set(MATHOPTSOLVERSCMAKE_USE_CPLEX ON)
endif()
FetchContent_Declare(
    mathoptsolverscmake
    GIT_REPOSITORY https://github.com/fontanf/mathoptsolverscmake.git
    GIT_TAG 56190725f424249a3acd8fce3ff50e08fe675cc6)
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../mathoptsolverscmake/")
FetchContent_MakeAvailable(mathoptsolverscmake)

# Fetch fontanf/stablesolver.
if(COLORINGSOLVER_USE_CBC)
    set(STABLESOLVER_USE_CBC ON)
endif()
if(COLORINGSOLVER_USE_CPLEX)
    set(STABLESOLVER_USE_CPLEX ON)
endif()
FetchContent_Declare(
    stablesolver
    GIT_REPOSITORY https://github.com/fontanf/stablesolver.git
    GIT_TAG 5c11e705937f2ac2e442ac17ae0e5d2c249cebc8)
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../stablesolver/")
FetchContent_MakeAvailable(stablesolver)

# Fetch fontanf/columngenerationsolver.
FetchContent_Declare(
    columngenerationsolver
    GIT_REPOSITORY https://github.com/fontanf/columngenerationsolver.git
    GIT_TAG 32561f17c6fa7100b2dedb5d397dab1a03c9db2c)
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../columngenerationsolver/")
FetchContent_MakeAvailable(columngenerationsolver)
