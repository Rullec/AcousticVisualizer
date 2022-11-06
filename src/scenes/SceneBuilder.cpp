#include "SceneBuilder.h"
#include "DrawScene.h"
#include "DrawSceneImGUI.h"
#include "utils/JsonUtil.h"
#include "utils/LogUtil.h"

cDrawScenePtr cSceneBuilder::BuildDrawScene(bool enable_draw_imgui /*= false */)
{
    cDrawScenePtr scene = nullptr;
    if (enable_draw_imgui == false)
    {
        scene = std::make_shared<cDrawScene>();
    }
    else
    {
        scene = std::make_shared<cDrawSceneImGui>();
    }
    return scene;
}
std::shared_ptr<cSimScene>
cSceneBuilder::BuildSimScene(const std::string config_file)
{
    Json::Value root;
    cJsonUtil::LoadJson(config_file, root);
    std::string type = cJsonUtil::ParseAsString("scene_type", root);
    eSceneType scheme = cSimScene::BuildSceneType(type);
    std::shared_ptr<cSimScene> scene = nullptr;
    switch (scheme)
    {
    case eSceneType::SCENE_SIM:
        scene = std::make_shared<cSimScene>();
        break;
    default:
        SIM_ERROR("unsupported sim scene {}", type);
        break;
    }
    
    return scene;
}