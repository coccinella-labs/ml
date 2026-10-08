cmake_minimum_required(VERSION 3.15)
project(DistributedMLFramework CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Optimized flags for ARM64/edge
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-O3 -march=native -ffast-math)
endif()

# Find packages
find_package(MPI REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread filesystem log)
find_package(GTest QUIET)

include_directories(
    ${MPI_INCLUDE_PATH}
    ${EIGEN3_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    include
)

# Source files
set(SOURCES
    src/distributed_trainer.cpp
    src/task_manager.cpp
    src/performance_tracker.cpp
    src/dashboard_server.cpp
    src/main.cpp
)

add_executable(distributed_ml ${SOURCES})

# Link libraries
target_link_libraries(distributed_ml
    ${MPI_LIBRARIES}
    Eigen3::Eigen
    cpprestsdk::cpprest
    nlohmann_json::nlohmann_json
    ${Boost_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Boost.Log macros
target_compile_definitions(distributed_ml PRIVATE BOOST_LOG_DYN_LINK)

# Enable testing if tests exist
enable_testing()
file(GLOB TEST_SOURCES "tests/*.cpp")
if(TEST_SOURCES)
    add_executable(unit_tests
        ${TEST_SOURCES}
        src/distributed_trainer.cpp
        src/task_manager.cpp
        src/performance_tracker.cpp)
    target_link_libraries(unit_tests
        ${MPI_LIBRARIES}
        Eigen3::Eigen
        cpprestsdk::cpprest
        nlohmann_json::nlohmann_json
        ${Boost_LIBRARIES}
        # GTest::gtest, not gtest_main: tests/test_main.cpp supplies main() so
        # that MPI lifetime is explicit rather than implicit in gtest_main.
        GTest::gtest
        OpenSSL::SSL
        OpenSSL::Crypto)
    target_include_directories(unit_tests PRIVATE include)

    # Single-process entry. The DistributedCoordinator tests are excluded here:
    # in a one-rank world their assertions hold trivially, so including them
    # would report a green that proves nothing.
    add_test(NAME unit_tests COMMAND unit_tests --gtest_filter=-DistributedCoordinator.*)

    # Distributed entry. The coordinator tests assert a three-rank world, so
    # they must not run unless mpiexec is available.
    find_program(MPIEXEC_EXECUTABLE mpiexec)
    if(MPIEXEC_EXECUTABLE)
        add_test(NAME unit_tests_distributed
                 COMMAND ${MPIEXEC_EXECUTABLE} -n 3
                         $<TARGET_FILE:unit_tests>
                         --gtest_filter=DistributedCoordinator.*)
        # A hung collective would otherwise stall the whole CI job.
        set_tests_properties(unit_tests_distributed PROPERTIES TIMEOUT 120)
    else()
        message(WARNING "mpiexec not found; the distributed coordinator tests will not run")
    endif()
endif()

install(TARGETS distributed_ml DESTINATION bin)
