cmake_minimum_required(VERSION 3.20)
project(rubiks_cube_solver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use nlohmann_json from FetchContent with policy fix
cmake_policy(SET CMP0158 NEW)
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# Disable non-portable bits/stdc++.h
if(NOT MSVC)
    add_compile_options(-Wall -Wextra -Wpedantic)
else()
    add_compile_options(/W3)
endif()

include_directories(Model PatternDatabases)

# Only main.cpp is compiled directly; model/solver files are included in main.cpp
add_executable(rubiks_cube_solver
    main.cpp
    Model/RubiksCube.cpp
    Model/RubiksCube.h
    Model/RubiksCube3dArray.cpp
    Model/RubiksCube1dArray.cpp
    Model/RubiksCubeBitboard.cpp
    Model/RubiksCubeBitboard.h
    PatternDatabases/NibbleArray.cpp
    PatternDatabases/NibbleArray.h
    PatternDatabases/PatternDatabase.cpp
    PatternDatabases/PatternDatabase.h
    PatternDatabases/PermutationIndexer.h
    PatternDatabases/CornerPatternDatabase.cpp
    PatternDatabases/CornerPatternDatabase.h
    PatternDatabases/CornerDBMaker.cpp
    PatternDatabases/CornerDBMaker.h
    PatternDatabases/RubiksMath.cpp
    PatternDatabases/RubiksMath.h
)

target_link_libraries(rubiks_cube_solver PRIVATE nlohmann_json::nlohmann_json)

# Install
install(TARGETS rubiks_cube_solver RUNTIME DESTINATION bin)