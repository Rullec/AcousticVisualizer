# -------------------enable openmp---------------------
if((CMAKE_MAJOR_VERSION GREATER_EQUAL 3) AND(CMAKE_MINOR_VERSION GREATER_EQUAL 20))
    cmake_policy(SET CMP0104 OLD)
endif()

if(NOT APPLE)
    list(APPEND CLOTH_SRC ${GPU_CLOTH_SRC})
endif()


include_directories("${CMAKE_CURRENT_BINARY_DIR}/../")
# message(FATAL_ERROR "${CMAKE_CURRENT_BINARY_DIR}/../")
set(
    SIM_SRC
    ${CLOTH_SRC}
    BaseObject.cpp
    KinematicBody.cpp
    AcousticBody.cpp
    KinematicBodyBuilder.cpp
    Perturb.cpp
    SimObjectBuilder.cpp
    AudioOutput.cpp
    AudioWave.cpp
    ../modal/ModalShape.cpp
    ../modal/ModalModel.cpp
    ../transfer/FMMTransferEval.cpp
    ../transfer/HelmholtzBasis.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../protobuf/sploosh.pb.cc
    ../io/MatrixIO.cpp
    ../generic/precision_type.cpp
    SNISR_DebugDraw.cpp
    AcousticManager.cpp
)

add_library(sim_lib ${SIM_SRC})
target_link_libraries(sim_lib utils_lib geo_lib ${MKL_LIBRARY} ${OpenMP_CXX_LIBRARIES}
    ${PROTOBUF_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${GSL_LIBRARIES}
    ${CMAKE_DL_LIBS})

target_compile_definitions(sim_lib PUBLIC "USE_MKL")
target_link_directories(sim_lib PUBLIC $ENV{MKLROOT}/lib/intel64)
target_include_directories(sim_lib PUBLIC ${MKL_INCLUDE_DIR})

# support pybind11 for acoustic
set (PYBIND11_PYTHON_VERSION "3.8.11")
pybind11_add_module(AcousticBinder AcousticBodyBind.cpp)
target_link_libraries(AcousticBinder PRIVATE sim_lib cam_lib utils_lib imgui_lib)
set_target_properties(AcousticBinder
    PROPERTIES  OUTPUT_NAME "AcousticBinder"
     DEBUG_POSTFIX ""
    )
install(TARGETS AcousticBinder
        COMPONENT python
        LIBRARY
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )