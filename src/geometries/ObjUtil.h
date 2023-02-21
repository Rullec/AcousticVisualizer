#pragma once
#include "utils/DefUtil.h"
#include "utils/EigenUtil.h"
#include <string>
#include <vector>

struct tTriangle;
struct tEdge;
struct tVertex;
SIM_DECLARE_PTR(tTriangle);
SIM_DECLARE_PTR(tEdge);
SIM_DECLARE_PTR(tVertex);
SIM_DECLARE_STRUCT_AND_PTR(tMeshMaterialInfo);
/**
 * \brief           handle everything about obj
 */

class cObjUtil
{
public:
    static void LoadObj(const std::string &path,
                        std::vector<tVertexPtr> &mVertexArray,
                        std::vector<tEdgePtr> &mEdgeArray,
                        std::vector<tTrianglePtr> &mTriangleArray,
                        std::vector<tMeshMaterialInfoPtr> &mMatInfoArray);
    static void
    BuildPlaneGeometryData(const _FLOAT scale, const tVector4 &plane_equation,
                           std::vector<tVertexPtr> &mVertexArray,
                           std::vector<tEdgePtr> &mEdgeArray,
                           std::vector<tTrianglePtr> &mTriangleArray);

    static void BuildEdge(const std::vector<tVertexPtr> &mVertexArray,
                          std::vector<tEdgePtr> &mEdgeArray,
                          const std::vector<tTrianglePtr> &mTriangleArray);

// protected:
//     static void HandleDoubleFace(std::vector<tVertexPtr> &v_array,
//                                  std::vector<tTrianglePtr> &t_array);
};
