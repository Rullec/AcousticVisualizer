#pragma once
#include "utils/DefUtil.h"

SIM_DECLARE_CLASS_AND_PTR(cDrawSceneImGui);
SIM_DECLARE_CLASS_AND_PTR(cSimScene);
class cMainApp : public std::enable_shared_from_this<cMainApp>
{
public:
    cMainApp();
    virtual void Init(const std::string &conf_path, cDrawSceneImGuiPtr draw_scene,
                      cSimScenePtr sim_scene);
    virtual void Mainloop();
    virtual bool ShouldClose();
    virtual void InitGLFW();

protected:
    cDrawSceneImGuiPtr mDrawScene;
    cSimScenePtr mSimScene;
    virtual void CloseGLFW();
    virtual void PollEvents();
};
SIM_DECLARE_PTR(cMainApp);