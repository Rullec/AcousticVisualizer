add_subdirectory(./json)

if(UNIX)
    find_package(spdlog REQUIRED)
    if(NOT spdlog_FOUND)
        message(FATAL_ERROR "spdlog not found")
    endif()
endif()
add_library(utils_lib 
FileUtil.cpp
JsonUtil.cpp
LogUtil.cpp
MathUtil.cpp
MathUtil2.cpp
Rand.cpp
TimeUtil.cpp
RotUtil.cpp
EigenUtil.cpp
# SysUtil.cpp
# ObjUtil.cpp
# TetUtil.cpp
# DepthImageSaver.cpp
# MPIUtil.cpp
StringUtil.cpp
RenderUtil.cpp
ImGuiUtil.cpp
)

target_link_libraries(utils_lib JSON)

# spdlog lib version on win32
if(WIN32)
    target_link_libraries(utils_lib spdlog)
endif()

# only for linux
if(UNIX AND (NOT DEFINED APPLE))
    target_link_libraries(utils_lib stdc++fs)
endif()