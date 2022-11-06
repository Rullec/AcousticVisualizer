# Macros to build the targets
include( CMakeParseArguments )

macro(config_compiler_and_linker)
    if (CMAKE_CXX_COMPILER MATCHES ".*icpc$")
        #add_definitions(-wd981 -wd383 -wd444)
        if ( NOT USE_DEBUG )
            #set(CMAKE_CXX_FLAGS_RELEASE "-O3 -no-prec-div -xHost -opt-prefetch -unroll-aggressive -DNDEBUG")
            set(CMAKE_CXX_FLAGS_RELEASE "-O3 -no-prec-div -opt-prefetch -unroll-aggressive -DNDEBUG")
        endif ()

        set(OPENMP_LIB iomp5)
    endif ()
endmacro()

# Usage:
# 
# add_exe( name source1 [source2 ...]
#          [LINK_LIBRARIES lib1 ...]
#          [INCLUDE_DIRS dir1 ...]
#          [OUT_DIR "output dir"] )
function(add_exe name)
    cmake_parse_arguments(_exe "" "OUT_DIR" "LINK_LIBRARIES;INCLUDE_DIRS" ${ARGN})
    set(_exe_srcs ${_exe_UNPARSED_ARGUMENTS})
    message(STATUS "--- add a executable target ${name} ---")

    add_executable(${name} ${_exe_srcs})
    if (_exe_OUT_DIR)
        set_target_properties(${name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${_exe_OUT_DIR})
    endif()

    if (_exe_LINK_LIBRARIES)
        target_link_libraries(${name} ${_exe_LINK_LIBRARIES})
    endif()

    if (_exe_INCLUDE_DIRS)
        include_directories(${_exe_INCLUDE_DIRS})
    endif()
endfunction()


# find google protobuf library
include_directories(
    ${EIGEN_INCLUDE_DIR}
    ${PROTOBUF_INCLUDE_DIRS})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS sploosh.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ../protobuf/sploosh.proto)
# message(FATAL_ERROR ${PROTO_HDRS} " " ${PROTO_SRCS})
# -----------------------------------------------------------------------------
# generate python protobuf file
# set(PY_SRC "${BIN_DIR}/python/sploosh_pb2.py")
# get_filename_component(ABS_PROTO_FILE sploosh.proto ABSOLUTE)
# get_filename_component(ABS_PROTO_PATH ${ABS_PROTO_FILE} PATH)
# add_custom_command(
#     OUTPUT ${PY_SRC}
#     COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
#     ARGS --python_out ${BIN_DIR}/python -I ${ABS_PROTO_PATH} ${ABS_PROTO_FILE}
#     DEPENDS ${ABS_PROTO_FILE}
#     COMMENT "Running python protocol buffer compiler on ${ABS_PROTO_FILE}"
#     VERBATIM)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/../")
add_exe(gen_moments gen_moments.cpp
    ${PROTO_HDRS}   ${PROTO_SRCS}
    ../generic/precision_type.hpp
    ../generic/precision_type.cpp
    ${PY_SRC}
    LINK_LIBRARIES ${Boost_SYSTEM_LIBRARY}
                   ${Boost_FILESYSTEM_LIBRARY}
                   ${Boost_PROGRAM_OPTIONS_LIBRARY}
                   ${GSL_LIBRARIES}
                   ${PROTOBUF_LIBRARY}
                   utils_lib
    OUT_DIR ${BIN_DIR})
# add_exe( name source1 [source2 ...]
#          [LINK_LIBRARIES lib1 ...]
#          [INCLUDE_DIRS dir1 ...]
#          [OUT_DIR "output dir"] )