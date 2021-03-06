cmake_minimum_required(VERSION 2.8)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (POLICY CMP0058 )
    cmake_policy(SET CMP0058 OLD)
endif()

include(cmake/Util.cmake)

project(joint)

if (JOINT_TREAT_WARNINGS_AS_ERRORS)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    endif()
endif()

if (JOINT_COVERAGE)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(STATUS "Using gcov for test coverage analysis")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    else()
        message(WARNING "No C/C++ coverage analysis support")
    endif()
endif()

if (JOINT_NATIVE_FUNCTIONS_ALIGNMENT)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -falign-functions=${JOINT_NATIVE_FUNCTIONS_ALIGNMENT}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -falign-functions=${JOINT_NATIVE_FUNCTIONS_ALIGNMENT}")
    else()
        message(WARNING "No C/C++ aligned functions support")
    endif()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/buildsystems/cmake/")

include_directories(${CMAKE_SOURCE_DIR}/core)
include_directories(${CMAKE_SOURCE_DIR}/devkit)

add_subdirectory(joint-gen)
add_subdirectory(core)
add_subdirectory(devkit)
add_subdirectory(bindings)
add_subdirectory(test)
add_subdirectory(examples)
add_subdirectory(benchmarks)

add_custom_target(
    check_whitespaces ALL
    COMMAND "${CMAKE_SOURCE_DIR}/cmake/py_scripts/validate_whitespaces.py" --dir "${CMAKE_SOURCE_DIR}"
)

################################################################################

set(CPACK_GENERATOR "RPM;DEB")
set(CPACK_RPM_COMPONENT_INSTALL "ON")
set(CPACK_DEB_COMPONENT_INSTALL "ON")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dmitry Koplyarov")
include(CPack)
