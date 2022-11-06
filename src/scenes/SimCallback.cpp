#include "SimCallback.h"
#include "scenes/SimScene.h"

extern cSimScenePtr gSimScene;

void cSimCallback::UpdateSimImGui()
{
    if (gSimScene)
        gSimScene->UpdateImGui();
}

void cSimCallback::UpdateSimPerturbPos(const tVector4 &ori,
                                       const tVector4 &dir)
{
    if (gSimScene)
        gSimScene->UpdatePerturbPos(ori, dir);
}

void cSimCallback::CursorMove(int xpos, int ypos)
{
    if (gSimScene)
        gSimScene->CursorMove(xpos, ypos);
}

void cSimCallback::CreatePerturb(tRayPtr ray)
{
    if (gSimScene)
        gSimScene->CreatePerturb(ray);
}
void cSimCallback::ReleasePerturb()
{
    if (gSimScene)
        gSimScene->ReleasePerturb();
}
void cSimCallback::MouseButton(int a, int b, int c)
{
    if (gSimScene)
        gSimScene->MouseButton(a, b, c);
}

void cSimCallback::Key(int a, int b, int c, int d)
{
    if (gSimScene)
        gSimScene->Key(a, b, c, d);
}