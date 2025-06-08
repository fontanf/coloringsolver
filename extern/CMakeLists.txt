# Enable FetchContent.
include(FetchContent)

# Fetch boost.
set(BOOST_INCLUDE_LIBRARIES thread filesystem system program_options dynamic_bitset)
set(BOOST_ENABLE_CMAKE ON)
include(FetchContent)
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(Boost)

# Fetch fontanf/optimizationtools.
FetchContent_Declare(
    optimizationtools
    GIT_REPOSITORY https://github.com/fontanf/optimizationtools.git
    GIT_TAG 65c8e11224aac1cf6e6e33c040b6b895341ad156
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../optimizationtools/"
    EXCLUDE_FROM_ALL)
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
    GIT_TAG 4472814a28a40f4d861ccd757e140835c3a31bd0
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../mathoptsolverscmake/"
    EXCLUDE_FROM_ALL)
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
    GIT_TAG 3c374c789c766111ac953e2a46d21881f5841cd0
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../stablesolver/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(stablesolver)

# Fetch fontanf/columngenerationsolver.
FetchContent_Declare(
    columngenerationsolver
    GIT_REPOSITORY https://github.com/fontanf/columngenerationsolver.git
    GIT_TAG c661bbd0f05e32161b07db2a28484dbb12d60c10
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../columngenerationsolver/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(columngenerationsolver)
