#pragma once
#include "geometries/Primitives.h"
#include "utils/DefUtil.h"
#include <memory>

/**
 * \brief               do raycasting for the triangle-based scene
 */
SIM_DECLARE_CLASS_AND_PTR(CameraBase);
SIM_DECLARE_CLASS_AND_PTR(cBaseObject);
SIM_DECLARE_STRUCT_AND_PTR(tRay);
namespace Json
{
class Value;
};

class cRaycaster : std::enable_shared_from_this<cRaycaster>
{
public:
    struct tRaycastResult
    {
        tRaycastResult();
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
        // object pointer
        cBaseObjectPtr mObject; // casted object pointer

        // local triangle id on this object
        int mLocalTriangleId;
        tVector4 mIntersectionPoint;
    };
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    explicit cRaycaster();
    virtual void Init(const Json::Value &conf);
    // virtual void AddResources(const std::vector<tTriangle *> triangles,
    //                           const std::vector<tVertexPtr > vertices);
    virtual void AddResources(cBaseObjectPtr object);
    tRaycastResult RayCast(const tRayPtr ray) const;
    virtual void CalcDepthMap(const tMatrix2i &cast_range_window, int height,
                              int width, CameraBasePtr camera,
                              std::string path);
    static void CalcCastWindowSize(const tMatrix2i &cast_range_window,
                                   int &window_width, int &window_height,
                                   tVector2i &st);
    void Update(const std::vector<cBaseObjectPtr> &object_array);

protected:
    std::vector<cBaseObjectPtr> mObjects;
    std::vector<std::vector<tTrianglePtr>> mTriangleArray_lst;
    std::vector<std::vector<tVertexPtr>> mVertexArray_lst;
};