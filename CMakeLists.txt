# Created by Alex
cmake_minimum_required(VERSION 3.13)
project(ecompiler)
set(CMAKE_CXX_STANDARD 14)
if (MSVC)
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE)
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif(MSVC)
set(ecompiler_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE INTERNAL "")
aux_source_directory(${ecompiler_include_dirs} sources)
include_directories(${ecompiler_include_dirs})
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(ecompiler_libs all)

add_subdirectory(third_party)

add_executable(ecompiler ${sources})
target_include_directories(ecompiler PUBLIC ${ecompiler_include_dirs})
target_link_libraries(ecompiler ${ecompiler_libs})
target_link_libraries(ecompiler nlohmann_json::nlohmann_json)
