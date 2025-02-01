cmake_minimum_required(VERSION 3.15)
project(DistributedMLFramework CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Verbose compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Homebrew paths for macOS
list(APPEND CMAKE_PREFIX_PATH 
    "/opt/homebrew/opt/open-mpi"
    "/opt/homebrew/opt/opencv"
    "/opt/homebrew/opt/eigen"
    "/opt/homebrew/opt/cpprestsdk"
    "/opt/homebrew/opt/nlohmann-json"
    "/opt/homebrew/opt/boost"
)

# Find required packages
find_package(MPI REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED COMPONENTS 
    system 
    thread 
    filesystem 
    log 
    log_setup
)

# Include directories
include_directories(
    ${MPI_INCLUDE_PATH}
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    include
)

# Source files
set(SOURCES
    src/distributed_trainer.cpp
    src/task_manager.cpp
    src/performance_tracker.cpp
    dashboard/dashboard_server.cpp
    src/main.cpp
)

# Executable
add_executable(distributed_ml_app ${SOURCES})

# Link libraries
target_link_libraries(distributed_ml_app 
    ${MPI_LIBRARIES}
    ${OpenCV_LIBS}
    Eigen3::Eigen
    cpprestsdk::cpprest
    nlohmann_json::nlohmann_json
    ${Boost_LIBRARIES}
)

# Compiler flags (moved after target definition)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set_target_properties(distributed_ml_app PROPERTIES
        COMPILE_FLAGS "-Wno-deprecated-declarations -Wno-unused-parameter"
    )
endif()

# Define preprocessor macros for Boost.Log
target_compile_definitions(distributed_ml_app PRIVATE
    BOOST_LOG_DYN_LINK
)

# Enable testing
enable_testing()

# Install
install(TARGETS distributed_ml_app DESTINATION bin)
