cmake_minimum_required(VERSION 3.27)

project(avm VERSION 0.1.0 LANGUAGES C CXX)

set(AVM_SOURCES
    "src/main.cpp"
)

add_executable(avm ${AVM_SOURCES})
set_target_properties(avm PROPERTIES
    CXX_STANDARD 17
)