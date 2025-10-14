# Enable FetchContent.
include(FetchContent)

# Fetch boost.
if(COLORINGSOLVER_BUILD_MAIN OR COLORINGSOLVER_BUILD_TEST)
    set(BOOST_INCLUDE_LIBRARIES thread filesystem system program_options dynamic_bitset)
    set(BOOST_ENABLE_CMAKE ON)
    include(FetchContent)
    FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
        EXCLUDE_FROM_ALL)
    FetchContent_MakeAvailable(Boost)
endif()

# Fetch googletest.
if(COLORINGSOLVER_BUILD_TEST)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF)
    FetchContent_MakeAvailable(googletest)
endif()

# Fetch fontanf/optimizationtools.
set(OPTIMIZATIONTOOLS_BUILD_TEST OFF)
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
    GIT_TAG f03fa63814eae3a253675ed14dd618a8a0d19a11
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../mathoptsolverscmake/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(mathoptsolverscmake)

# Fetch fontanf/stablesolver.
set(STABLESOLVER_BUILD_MAIN OFF)
set(STABLESOLVER_BUILD_TEST OFF)
if(COLORINGSOLVER_USE_CBC)
    set(STABLESOLVER_USE_CBC ON)
endif()
if(COLORINGSOLVER_USE_CPLEX)
    set(STABLESOLVER_USE_CPLEX ON)
endif()
FetchContent_Declare(
    stablesolver
    GIT_REPOSITORY https://github.com/fontanf/stablesolver.git
    GIT_TAG 673f47a1daa32d5eb33a09083bb15a87b7c2fb65
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../stablesolver/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(stablesolver)

# Fetch fontanf/columngenerationsolver.
set(COLUMNGENERATIONSOLVER_BUILD_TEST OFF)
set(COLUMNGENERATIONSOLVER_BUILD_EXAMPLES OFF)
FetchContent_Declare(
    columngenerationsolver
    GIT_REPOSITORY https://github.com/fontanf/columngenerationsolver.git
    GIT_TAG ccecf31494997c7af64c850871f8ecf48ae1dade
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../columngenerationsolver/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(columngenerationsolver)
