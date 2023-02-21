#include "app/MainApp.h"
#include "cxxopts.hpp"
#include "scenes/SceneBuilder.h"
#include "utils/JsonUtil.h"
#include "utils/LogUtil.h"
#include "utils/MathUtil.h"
#include "utils/json/json.h"
#include "scenes/DrawSceneImGUI.h"
cDrawScenePtr gDrawScene = nullptr;
cSimScenePtr gSimScene = nullptr;
extern int gWindowWidth, gWindowHeight;

void ParseArg(int argc, char *argv[], std::string &config_path);

int main(int argc, char **argv)
{
    cMathUtil::SeedRand(0);

    std::string conf = "";
    ParseArg(argc, argv, conf);
    std::string app_type = "";
    {
        Json::Value root;
        cJsonUtil::LoadJson(conf, root);
        app_type = cJsonUtil::ParseAsString("app_type", root);
    }

    cMainAppPtr main_scene = std::make_shared<cMainApp>();

    main_scene->InitGLFW();

    gSimScene = cSceneBuilder::BuildSimScene(conf);
    gSimScene->Init(conf);
    gDrawScene = cSceneBuilder::BuildDrawScene(true);
    gDrawScene->Init(conf);

    main_scene->Init(conf,
                     std::dynamic_pointer_cast<cDrawSceneImGui>(gDrawScene),
                     gSimScene);
    main_scene->Mainloop();
    return 0;
}

void ParseArg(int argc, char *argv[], std::string &config_path)
{
    try
    {
        cxxopts::Options options(argv[0], " - simple acoustic simulator");
        options.positional_help("[optional args]").show_positional_help();

        options.add_options()("conf", "config path",
                              cxxopts::value<std::string>());

        options.parse_positional({"conf"});
        auto result = options.parse(argc, argv);

        if (result.count("conf"))
        {
            config_path = result["conf"].as<std::string>();
            std::cout << "saw param config = " << config_path << std::endl;
        }
    }
    catch (const cxxopts::OptionException &e)
    {
        std::cout << "[error] when parsing, " << e.what() << std::endl;
        exit(1);
    }
    // SIM_INFO("conf path {}, enable imgui {}", config_path, disable_imgui);
    SIM_INFO("conf path {}", config_path);
    Json::Value root;
    cJsonUtil::LoadJson(config_path, root);
    gWindowWidth = cJsonUtil::ParseAsInt("window_width", root);
    gWindowHeight = cJsonUtil::ParseAsInt("window_height", root);
}