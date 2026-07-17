cmake_minimum_required(VERSION 3.30)

project(raft VERSION 1.0)

# Enforce C++20 standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(LLVM REQUIRED CONFIG)

# message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
# message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# for LLVM
include_directories(src)
# separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
# add_definitions(${LLVM_DEFINITIONS_LIST})

# Define source files
set(SOURCES
    src/main.cpp
    src/TypeChecker/TypeChecker.cpp
    src/Interpreter/Interpreter.cpp
    src/Lexer/lexer.cpp
    src/Parser/parser.cpp
    src/Util/token.cpp
)

# Create the executable target
add_executable(raft ${SOURCES})

# Find the libraries that correspond to the LLVM components
# that we wish to use
# llvm_map_components_to_libnames(llvm_libs support core irreader)

# Link against the libraries built in the subdirectories
# target_link_libraries(raft ${llvm_libs})

# Set output directory to bin
set_target_properties(raft PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)