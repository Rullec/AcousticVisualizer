#pragma once
#include "scenes/DrawScene.h"
#include "scenes/SimScene.h"
#include "utils/DefUtil.h"
#include <memory>
#include <string>

class cSceneBuilder
{
public:
    static cDrawScenePtr BuildDrawScene(bool enable_draw_imgui = false);
    static cSimScenePtr BuildSimScene(const std::string config_file);
};