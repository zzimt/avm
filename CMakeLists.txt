cmake_minimum_required(VERSION 3.27)

project(avm VERSION 0.1.0 LANGUAGES C CXX)

set(AVM_SOURCES
    "src/main.cpp"
)

add_executable(avm ${AVM_SOURCES})
set_target_properties(avm PROPERTIES
    CXX_STANDARD 17
)

target_include_directories(avm PRIVATE "vendor/rapidhash/include")

add_subdirectory("vendor/abseil-cpp")

target_link_libraries(avm absl::flat_hash_map m)