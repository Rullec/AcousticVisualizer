# -------------------enable openmp---------------------
if((CMAKE_MAJOR_VERSION GREATER_EQUAL 3) AND (CMAKE_MINOR_VERSION GREATER_EQUAL 20))
    cmake_policy(SET CMP0104 OLD)
endif()


if(NOT APPLE)
    list(APPEND CLOTH_SRC ${GPU_CLOTH_SRC})
endif()

set(
    SIM_SRC
    ${CLOTH_SRC}
    BaseObject.cpp
    KinematicBody.cpp
    AcousticBody.cpp
    KinematicBodyBuilder.cpp
    Perturb.cpp
    SimObjectBuilder.cpp
)

add_library(sim_lib ${SIM_SRC})
target_link_libraries(sim_lib utils_lib geo_lib)