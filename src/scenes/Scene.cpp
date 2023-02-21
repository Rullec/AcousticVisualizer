#include "Scene.h"
cScene::cScene()
{
    Reset();
}
cScene::~cScene() {}

void cScene::Update(_FLOAT dt)
{
    mCurdt = dt;
    mCurTime += dt;
}

void cScene::Reset()
{
    mCurTime = 0;
}