#include "SimScene.h"
// #include "sim/collision/CollisionDetecter.h"
#include "geometries/Primitives.h"
#include "geometries/Triangulator.h"
#include "imgui.h"
#include "scenes/SimStateMachine.h"
#include "sim/KinematicBody.h"
#include "sim/Perturb.h"
#include "sim/SimObjectBuilder.h"
#include "utils/ColorUtil.h"
#include "utils/JsonUtil.h"
#include "utils/LogUtil.h"
#include "utils/MathUtil.h"
#include "utils/RenderUtil.h"
#include "utils/json/json.h"
#include <iostream>

std::string gSceneTypeStr[eSceneType::NUM_OF_SCENE_TYPES] = {"sim", "diff_sim"};

eSceneType cSimScene::BuildSceneType(const std::string &str)
{
    int i = 0;
    for (i = 0; i < eSceneType::NUM_OF_SCENE_TYPES; i++)
    {
        // std::cout << gSceneTypeStr[i] << std::endl;
        if (str == gSceneTypeStr[i])
        {
            break;
        }
    }

    SIM_ASSERT(i != eSceneType::NUM_OF_SCENE_TYPES);
    return static_cast<eSceneType>(i);
}

cSimScene::cSimScene()
{
    // mTriangleArray.clear();
    // mEdgeArray.clear();
    // mVertexArray.clear();

    mPerturb = nullptr;
    mSimStateMachine = std::make_shared<tSimStateMachine>();
    mSceneType = eSceneType::SCENE_SIM;
}

eSceneType cSimScene::GetSceneType() const { return this->mSceneType; }

void cSimScene::Init(const std::string &conf_path)
{
    // 1. load config
    Json::Value root;
    cJsonUtil::LoadJson(conf_path, root);

    mEnableProfiling =
        cJsonUtil::ParseAsBool(cSimScene::ENABLE_PROFLINE_KEY, root);

    mEnableCollisionDetection =
        cJsonUtil::ParseAsBool(cSimScene::ENABLE_COLLISION_DETECTION_KEY, root);
    BuildObjects(cJsonUtil::ParseAsValue(cSimScene::OBJECT_LIST_KEY, root));

    CreateCollisionDetecter();

    InitDrawBuffer();
    InitRaycaster(root);

    mSimStateMachine->SimulatorInitDone(
        cJsonUtil::ParseAsBool("pause_at_first", root) == true
            ? eSimState::SIMSTATE_PAUSE
            : eSimState::SIMSTATE_RUN);
}
void cSimScene::BuildObjects(const Json::Value &obj_conf_)
{
    // 1. parse the number of obstacles
    Json::Value obj_conf = obj_conf_;
    int num_of_obstacles = obj_conf.size();
    SIM_ASSERT(num_of_obstacles == obj_conf.size());
    for (int i = 0; i < num_of_obstacles; i++)
    {
        auto obs = BuildSimObj(obj_conf[i], mObjectList.size());
        // auto obs = BuildKinematicBody(obj_conf[i], GetNumOfObjects());
        mObjectList.push_back(obs);
    }
}

/**
 * \breif       save current scene (obstacles to objes)
 */
#include "geometries/ObjExport.h"
void cSimScene::SaveCurrentScene()
{
    for (auto &x : this->mObjectList)
    {
        cObjExporter::ExportObj(x->GetObjName() + ".obj", x->GetVertexArray(),
                                x->GetTriangleArray());
    }
}

void cSimScene::InitDrawBuffer() { UpdateRenderingResource(); }

/**
 * \brief           Init the raycasting strucutre
 */
#include "scenes/Raycaster.h"
void cSimScene::InitRaycaster(const Json::Value &conf)
{
    mRaycaster = std::make_shared<cRaycaster>();
    mRaycaster->Init(conf);
    for (auto &x : mObjectList)
        mRaycaster->AddResources(x);
    std::cout << "[debug] add resources to raycaster done, num of objects = "
              << mObjectList.size() << std::endl;
}
/**
 * \brief           Update the simulation procedure
 */
#include "utils/TimeUtil.hpp"
static int frame = 0;
void cSimScene::Update(_FLOAT delta_time)
{

    if (mSimStateMachine->IsRunning() == true)
    {
        // std::cout << "--------------frame " << frame++ << "-----------\n";
        // float default_dt = mIdealDefaultTimestep;
        // if (delta_time < default_dt)
        //     default_dt = delta_time;

        // printf("[debug] sim scene update cur time = %.4f\n", mCurTime);
        cScene::Update(delta_time);

        UpdateObjects();
        PerformCollisionDetection();
        UpdateRaycaster();
        // clear force
        // apply ext force
        // update position
    }
    mSimStateMachine->SimulationForwardOneFrameDone();
}

/**
 * \brief           update obstacles
 */
void cSimScene::UpdateObjects()
{

    // 1. update perturb on objects if possible
    if (mPerturb != nullptr)
    {
        // ApplyUserPerturbForceOnce
        mPerturb->mObject->ApplyUserPerturbForceOnce(mPerturb);
    }

    // 2. update objects

    for (auto &obs : this->mObjectList)
    {
        obs->Update(mCurdt);
    }
}
/**
 * \brief       do (discrete) collision detection
 */
void cSimScene::PerformCollisionDetection() {}
/**
 * \brief           Reset the whole scene
 */
void cSimScene::Reset()
{
    cScene::Reset();
    ReleasePerturb();
    ClearForce();
    for (auto &x : mObjectList)
        x->Reset();
}

/**
 * \brief           Get number of vertices
 */
int cSimScene::GetNumOfVertices() const
{
    int num_of_vertices = 0;
    for (auto &x : mObjectList)
    {
        num_of_vertices += x->GetNumOfVertices();
    }
    return num_of_vertices;
}

/**
 * \brief       clear all forces
 */
void cSimScene::ClearForce() {}

int cSimScene::GetNumOfFreedom() const { return GetNumOfVertices() * 3; }

int cSimScene::GetNumOfDrawEdges() const
{
    int num_of_edges = 0;
    for (auto &x : mObjectList)
    {
        num_of_edges += x->GetNumOfEdges();
    }
    return num_of_edges;
}

int cSimScene::GetNumOfTriangles() const
{
    int num_of_triangles = 0;
    for (auto &x : mObjectList)
    {
        num_of_triangles += x->GetNumOfTriangles();
    }
    return num_of_triangles;
}
static std::string gUpdateRenderingResourceInfo = "";
#include "utils/ProfUtil.h"
void cSimScene::UpdateRenderingResource()
{
    cProfUtil::ClearRoot("render_res");
    cProfUtil::Begin("render_res");

    mRenderResource.clear();
    for (auto &obj : mObjectList)
    {
        cProfUtil::Begin("render_res/" + obj->GetObjName());
        obj->UpdateRenderingResource();
        cProfUtil::End("render_res/" + obj->GetObjName());
        auto cur_resource = obj->GetRenderingResource();
        mRenderResource.insert(mRenderResource.end(), cur_resource.begin(),
                               cur_resource.end());
    }
    cProfUtil::End("render_res");
    gUpdateRenderingResourceInfo = cProfUtil::GetTreeDesc("render_res");
    // CalcEdgesDrawBuffer();
    // CalcTriangleDrawBuffer();
    // CalcPointDrawBuffer();
    // printf("[debug] sim scene tri %d edge %d point %d\n",
    //        this->mRenderResource->mTriangleBuffer.mNumOfEle,
    //        this->mRenderResource->mLineBuffer.mNumOfEle,
    //        this->mRenderResource->mPointBuffer.mNumOfEle);
}

cSimScene::~cSimScene()
{
    // for (auto x : mVertexArray)
    //     delete x;
    // mVertexArray.clear();
    // for (auto &x : mTriangleArray)
    //     delete x;
    // mTriangleArray.clear();
    // for (auto &x : mEdgeArray)
    //     delete x;
    // mEdgeArray.clear();
}

/**
 * \brief               Event response (add perturb)
 */
void cSimScene::CursorMove(int xpos, int ypos) {}

void cSimScene::UpdatePerturbPos(const tVector4 &camera_pos,
                                 const tVector4 &dir)
{
    if (mPerturb != nullptr)
    {
        mPerturb->UpdatePerturbPos(camera_pos, dir);
    }
}
/**
 * \brief               Event response (add perturb)
 */

void cSimScene::MouseButton(int button, int action, int mods) {}

#include "GLFW/glfw3.h"
void cSimScene::Key(int key, int scancode, int action, int mods)
{
    // std::cout << "[sim scene] key = " << key << std::endl;
    mSimStateMachine->Key(key, scancode, action, mods);
    switch (key)
    {
    case GLFW_KEY_S:
        std::cout << "[draw scene] key S, save now\n";
        SaveCurrentScene();
        break;
    case GLFW_KEY_R:
        Reset();
        break;
    }
}

bool cSimScene::CreatePerturb(tRayPtr ray)
{
    SIM_ASSERT(mRaycaster != nullptr);

    cRaycaster::tRaycastResult res = mRaycaster->RayCast(ray);
    if (res.mObject == nullptr)
    {
        return false;
    }
    else
    {
        std::cout << "[debug] add perturb on triangle " << res.mLocalTriangleId
                  << std::endl;
    }

    // 2. we have a triangle to track
    SIM_ASSERT(mPerturb == nullptr);

    mPerturb = new tPerturb();

    mPerturb->mObject = res.mObject;
    mPerturb->mAffectedTriId = res.mLocalTriangleId;
    // std::cout << "[debug] affect id = " << res.mLocalTriangleId << std::endl;
    const auto &ver_array = mPerturb->mObject->GetVertexArray();
    const auto &tri_array = mPerturb->mObject->GetTriangleArray();

    auto cur_t = tri_array[res.mLocalTriangleId];
    // printf("[cast] tri size %d v size %d, tid %d, v0 %d v1 %d v2 %d\n",
    //        tri_array.size(), ver_array.size(), res.mLocalTriangleId,
    //        cur_t->mId0, cur_t->mId1, cur_t->mId2);
    mPerturb->mBarycentricCoords =
        cMathUtil::CalcBarycentric(
            res.mIntersectionPoint, ver_array[cur_t->mId0]->mPos,
            ver_array[cur_t->mId1]->mPos, ver_array[cur_t->mId2]->mPos)
            .segment(0, 3);
    // std::cout << cur_t->mColor.transpose() << std::endl;
    // std::cout << cur_t->mNormal.transpose() << std::endl;
    // std::cout << cur_t->mEId0 << std::endl;
    // std::cout << cur_t->mEId1 << std::endl;
    // std::cout << cur_t->mEId2 << std::endl;
    // std::cout << "[perturb] intersection pt (from raycast) = "
    //           << res.mIntersectionPoint.transpose() << std::endl;
    // std::cout << "[perturb] bary = " <<
    // mPerturb->mBarycentricCoords.transpose()
    //           << std::endl;

    tVector4 restore_intersection_pt =
        ver_array[cur_t->mId0]->mPos * mPerturb->mBarycentricCoords[0] +
        ver_array[cur_t->mId1]->mPos * mPerturb->mBarycentricCoords[1] +
        ver_array[cur_t->mId2]->mPos * mPerturb->mBarycentricCoords[2];
    // std::cout << "[perturb] restore_intersection_pt = "
    //           << restore_intersection_pt.transpose() << std::endl;

    // std::cout
    //     << "uv = "
    //     << ver_array[cur_t->mId0]->muv_simple.transpose()
    //     << " vid = " << cur_t->mId0 << std::endl;
    SIM_ASSERT(mPerturb->mBarycentricCoords.hasNaN() == false);
    mPerturb->InitTangentRect(-1 * ray->mDir);
    mPerturb->UpdatePerturbPos(ray->mOrigin, ray->mDir);

    // change the color
    // mPerturb->mObject->ChangeTriangleColor(res.mLocalTriangleId,
    //                                        ColorShoJoHi.segment(0, 3));
    return true;
}
void cSimScene::ReleasePerturb()
{
    if (mPerturb != nullptr)
    {
        // restore the color

        // mPerturb->mObject->ChangeTriangleColor(mPerturb->mAffectedTriId,
        //                                        ColorBlue.segment(0, 3));
        // 1, 0);
        delete mPerturb;
        mPerturb = nullptr;
    }
}

#include "sim/KinematicBodyBuilder.h"

void cSimScene::CreateObstacle(const Json::Value &conf)
{

    // printf("[debug] create %d obstacle(s) done\n", mObstacleList.size());
    // exit(0);
}

// /**
//  * \brief                   Raycast the whole scene
//  * @param ray:              the given ray
//  * @param selected_tri:     a reference to selected triangle pointer
//  * @param selected_tri_id:  a reference to selected triangle id
//  * @param raycast_point:    a reference to intersection point
//  */
// void cSimScene::RayCastScene(const tRay *ray, cBaseObjectPtr casted_obj,
//                              int obj_triangle_id, tVector inter_point) const
// {
//     SIM_ASSERT(mRaycaster != nullptr);
//     cRaycaster::tRaycastResult res = mRaycaster->RayCast(ray);
//     casted_obj = res.mObject;
//     obj_triangle_id
// }

/**
 * \brief                   Collision Detection
 */
void cSimScene::CreateCollisionDetecter()
{
    if (mEnableCollisionDetection)
    {
    }
}

/**
 * \brief                   Get number of objects
 */
int cSimScene::GetNumOfObjects() const
{
    int num_of_objects = 0;
    // num_of_objects += mObstacleList.size();
    return num_of_objects;
}

/**
 * \brief                   Get obstacle list
 */
std::vector<cKinematicBodyPtr> cSimScene::GetObstacleList()
{
    // return this->mObstacleList;
    return {};
}

/**
 * \brief                   Is sim paused
 */
bool cSimScene::IsSimPaused() const
{
    return mSimStateMachine->IsRunning() == false;
}

/**
 * \brief                   Update imgui for simulation scene
 */
void cSimScene::UpdateImGui()
{
    auto cur_state = mSimStateMachine->GetCurrentState();
    auto name = tSimStateMachine::BuildStateStr(cur_state);
    // std::cout << "cSimScene::UpdateImGui, cur state = " << cur_state << "
    // name = " << name << std::endl;
    ImGui::Text("simulation state: %s", name.c_str());
    for (auto &obj : this->mObjectList)
    {
        obj->UpdateImGui();
    }
    // update vertices and triangle number
    int v_total = 0, t_total = 0;
    for (auto &obj : this->mObjectList)
    {
        int num_of_v = obj->GetNumOfVertices();
        int num_of_t = obj->GetNumOfTriangles();
        v_total += num_of_v;
        t_total += num_of_t;
        ImGui::Text("%s v %d t %d", obj->GetObjName().c_str(), num_of_v,
                    num_of_t);
    }
    ImGui::SameLine();
    ImGui::Text("total v %d t %d", v_total, t_total);
    ImGui::Text(gUpdateRenderingResourceInfo.c_str());
}

std::vector<cRenderResourcePtr> cSimScene::GetRenderResource()
{
    return mRenderResource;
}

std::vector<cBaseObjectPtr> cSimScene::GetObjList() const
{
    return this->mObjectList;
}

void cSimScene::RunSimulator() { mSimStateMachine->Run(); }

void cSimScene::UpdateRaycaster() { mRaycaster->Update(mObjectList); }