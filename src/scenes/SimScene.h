#pragma once
#include "Scene.h"
#include "utils/DefUtil.h"
#include "utils/EigenUtil.h"

namespace Json
{
class Value;
};

enum eSceneType
{
    SCENE_SIM = 0, // default simulation scene
    SCENE_DIFFSIM, // diff simulation scene
    // SCENE_ACOUSTIC, // acoustic simulation scene
    NUM_OF_SCENE_TYPES
};

struct tTriangle;
struct tPerturb;

SIM_DECLARE_CLASS_AND_PTR(cKinematicBody)
SIM_DECLARE_CLASS_AND_PTR(cBaseObject)
SIM_DECLARE_CLASS_AND_PTR(cRaycaster)
SIM_DECLARE_CLASS_AND_PTR(cCollisionDetecter)
SIM_DECLARE_CLASS_AND_PTR(cRenderResource)
SIM_DECLARE_STRUCT_AND_PTR(tSimStateMachine)
SIM_DECLARE_STRUCT_AND_PTR(tRay)

class cSimScene : public cScene
{
public:
    inline static const std::string ENABLE_PROFLINE_KEY = "enable_profiling",
                                    ENABLE_OBSTACLE_KEY = "enable_obstacle",
                                    OBJECT_LIST_KEY = "obj_list",
                                    ENABLE_COLLISION_DETECTION_KEY =
                                        "enable_collision_detection";

    cSimScene();
    ~cSimScene();
    virtual void Init(const std::string &conf_path) override;
    virtual void Update(_FLOAT dt) override;
    virtual void UpdateRenderingResource();
    virtual void Reset() override;
    static eSceneType BuildSceneType(const std::string &str);
    eSceneType GetSceneType() const;
    virtual bool CreatePerturb(tRayPtr ray);
    virtual void ReleasePerturb();
    virtual void UpdatePerturbPos(const tVector4 &camera_pos,
                                  const tVector4 &dir);
    virtual void CursorMove(int xpos, int ypos);
    virtual void MouseButton(int button, int action, int mods);
    virtual void Key(int key, int scancode, int action, int mods);
    void RayCastScene(const tRay *ray, tTriangle **selected_triangle,
                      int &selected_triangle_id,
                      tVector4 &ray_cast_position) const;
    virtual int GetNumOfObjects() const;
    virtual std::vector<cKinematicBodyPtr> GetObstacleList();
    virtual bool IsSimPaused() const;
    virtual void RunSimulator();
    virtual void UpdateImGui();
    virtual std::vector<cRenderResourcePtr> GetRenderResource();
    virtual std::vector<cBaseObjectPtr> GetObjList() const;

protected:
    tSimStateMachinePtr mSimStateMachine;
    tPerturb *mPerturb;
    eSceneType mSceneType;
    bool mEnableProfiling;
    bool mEnableObstacle; // using obstacle?
    bool mEnableCollisionDetection;
    std::vector<cBaseObjectPtr> mObjectList;
    cRaycasterPtr mRaycaster; // raycaster

    std::vector<cRenderResourcePtr> mRenderResource;

    // base methods
    void CalcDampingForce(const tVectorXf &vel, tVectorXf &damping) const;
    virtual void InitDrawBuffer();
    virtual void InitRaycaster(const Json::Value &conf);
    virtual void UpdateRaycaster();

    void ClearForce(); // clear all forces
    void SaveCurrentScene();

    virtual void BuildObjects(const Json::Value &obj_conf_path);
    virtual int GetNumOfVertices() const;
    virtual int GetNumOfFreedom() const;
    virtual int GetNumOfDrawEdges() const;
    virtual int GetNumOfTriangles() const;
    void CalcNodePositionVector(tVectorXf &pos) const;
    virtual void UpdateObjects();
    virtual void CreateObstacle(const Json::Value &conf);
    virtual void CreateCollisionDetecter();
    virtual void PerformCollisionDetection();
};

SIM_DECLARE_PTR(cSimScene);