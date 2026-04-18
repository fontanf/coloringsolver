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
if(SETCOVERINGSOLVER_BUILD_TEST)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF)
    FetchContent_MakeAvailable(googletest)
endif()

# Fetch fontanf/optimizationtools.
FetchContent_Declare(
    optimizationtools
    GIT_REPOSITORY https://github.com/fontanf/optimizationtools.git
    GIT_TAG 65c8e11224aac1cf6e6e33c040b6b895341ad156
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../optimizationtools/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(optimizationtools)

# Fetch fontanf/mathoptsolverscmake.
if(COLORINGSOLVER_USE_CLP)
    set(MATHOPTSOLVERSCMAKE_USE_CLP ON)
endif()
if(COLORINGSOLVER_USE_CBC)
    set(MATHOPTSOLVERSCMAKE_USE_CBC ON)
endif()
if(COLORINGSOLVER_USE_HIGHS)
    set(MATHOPTSOLVERSCMAKE_USE_HIGHS ON)
endif()
if(COLORINGSOLVER_USE_XPRESS)
    set(MATHOPTSOLVERSCMAKE_USE_XPRESS ON)
endif()
FetchContent_Declare(
    mathoptsolverscmake
    GIT_REPOSITORY https://github.com/fontanf/mathoptsolverscmake.git
    GIT_TAG c312e55d551edd57e98edfd24d4e2330abdcbbd2
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../mathoptsolverscmake/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(mathoptsolverscmake)

# Fetch fontanf/stablesolver.
set(STABLESOLVER_BUILD_MAIN OFF)
set(STABLESOLVER_BUILD_TEST OFF)
if(COLORINGSOLVER_USE_CBC)
    set(STABLESOLVER_USE_CBC ON)
endif()
if(COLORINGSOLVER_USE_HIGHS)
    set(STABLESOLVER_USE_HIGHS ON)
endif()
if(COLORINGSOLVER_USE_XPRESS)
    set(STABLESOLVER_USE_XPRESS ON)
endif()
FetchContent_Declare(
    stablesolver
    GIT_REPOSITORY https://github.com/fontanf/stablesolver.git
    GIT_TAG efab011b460c2675647fa1995011eb332ab9ec7d
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../stablesolver/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(stablesolver)

# Fetch fontanf/columngenerationsolver.
set(COLUMNGENERATIONSOLVER_BUILD_EXAMPLES OFF)
set(COLUMNGENERATIONSOLVER_BUILD_TEST OFF)
if(COLORINGSOLVER_USE_CLP)
    set(COLUMNGENERATIONSOLVER_USE_CLP ON)
endif()
if(COLORINGSOLVER_USE_HIGHS)
    set(COLUMNGENERATIONSOLVER_USE_HIGHS ON)
endif()
if(COLORINGSOLVER_USE_XPRESS)
    set(COLUMNGENERATIONSOLVER_USE_XPRESS ON)
endif()
FetchContent_Declare(
    columngenerationsolver
    GIT_REPOSITORY https://github.com/fontanf/columngenerationsolver.git
    GIT_TAG 556971624c92c23c961371ce5721ff9eb920a874
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../columngenerationsolver/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(columngenerationsolver)
