
set(JSON_SOURCES
    json/json_reader.cpp
    json/json_value.cpp
    json/json_writer.cpp
)

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
ProfUtil.cpp
RenderGrid.cpp
# SysUtil.cpp
# ObjUtil.cpp
# TetUtil.cpp
# DepthImageSaver.cpp
# MPIUtil.cpp
# RenderGrid.cpp
StringUtil.cpp
RenderUtil.cpp
# ImGuiUtil.cpp

${JSON_SOURCES}
)
target_include_directories(utils_lib PUBLIC ".")

# spdlog lib version on win32
if(WIN32)
    target_link_libraries(utils_lib PUBLIC spdlog)
endif()

# only for linux
if(UNIX AND (NOT DEFINED APPLE))
    target_link_libraries(utils_lib PRIVATE stdc++fs)
endif()

install(TARGETS utils_lib RUNTIME)