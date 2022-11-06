cmake_minimum_required(VERSION 3.10)

message("Generated with build types: ${CMAKE_BUILD_TYPE}")
set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")
project(ClothSim VERSION 1.0 LANGUAGES CXX)

# add_subdirectory(includes/pybind11)
# set(PYBIND11_INCLUDE_PATH "includes/pybind11/include")
# include_directories(PYBIND11_INCLUDE_PATH)
include_directories("${PROJECT_SOURCE_DIR}/includes")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/imgui")
include_directories("${PROJECT_SOURCE_DIR}/implot")

# set language standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_DEBUG_POSTFIX "d")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(configure_movecmd)

if(WIN32)
    link_directories(./lib)
endif()


if(WIN32)

    add_compile_options(
        
        $<$<CONFIG:RELEASE>: /MD>
        $<$<CONFIG:DEBUG>: /MDd>
    )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /wd4061 /wd4244 /wd4246 /wd4305 /wd4267 /wd4711 /wd4710 /wd4514 /wd4477 /W0 /MP /Zi")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
else()
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

# if(True)
#     message(STATUS "add address sanitizer")
#     if(WIN32)
#         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
#         SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /fsanitize=address")
#     else()
#         SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
#         SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
#     endif()
# endif()

# --------------------------
find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "vulkan not found")
endif()

set(Boost_ADDITIONAL_VERSIONS "1.51" "1.51.0")
find_package(Boost 1.50 REQUIRED 
    COMPONENTS "filesystem" "system" "program_options")

find_package(GSL REQUIRED)

find_package(OpenMP REQUIRED)
message("OpenMP_CXX_LIBRARIES: " ${OpenMP_CXX_LIBRARIES})

set(MKL_USE_STATIC_LIBS ON)
find_package(MKL REQUIRED)
message("MKL_LIBRARY " ${MKL_LIBRARY})
message("MKL PATH " $ENV{MKLROOT}/lib/intel64)

find_package(Protobuf REQUIRED VERSION 3.21.5)
message("Protobuf_VERSION = " ${Protobuf_VERSION})
message("Protobuf_INCLUDE_DIRS = " ${Protobuf_INCLUDE_DIRS})
message("Protobuf_LIBRARIES = " ${Protobuf_LIBRARIES})
message("Protobuf_PROTOC_LIBRARIES = " ${Protobuf_PROTOC_LIBRARIES})
message("Protobuf_LITE_LIBRARIES = " ${Protobuf_LITE_LIBRARIES})


add_subdirectory(./src/protobuf)
add_subdirectory(./imgui)
add_subdirectory(./implot)
add_subdirectory(./src/utils)
add_subdirectory(./src/geometries)
add_subdirectory(./src/sim)
add_subdirectory(./src/cameras)
add_subdirectory(./src/scenes)


set(my_libs ${scenes_lib} ${utils_lib} ${imgui_lib})

add_executable(main ./src/main.cpp ./src/app/MainApp.cpp)
target_link_libraries(main PUBLIC ${libs} scenes_lib utils_lib geo_lib)
target_include_directories(main PUBLIC ${PROJECT_BINARY_DIR})
if(WIN32)
    move_release_to_src(main)
else()
    install(TARGETS main
                RUNTIME
                DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
        )
endif()