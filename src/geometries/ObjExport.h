#pragma once
#include "utils/DefUtil.h"
#include <string>
#include <vector>
/**
 * \brief           export object (with no texture and texture coords)
 */
// class cObjExporter
// {
// public:
//     static bool ExportObj();
//     // static bool ExportObj;
// };

// class cObjExporter
// {
// public:
//     static bool ExportObj(std::string export_path,
//                           const std::vector<tVertexPtr > &vertices_array,
//                           const std::vector<tTriangle *> &triangles_array);
// };
SIM_DECLARE_STRUCT_AND_PTR(tVertex);
SIM_DECLARE_STRUCT_AND_PTR(tEdge);
SIM_DECLARE_STRUCT_AND_PTR(tTriangle);
class cObjExporter
{
public:
    static bool ExportObj(std::string export_path,
                          const std::vector<tVertexPtr > &vertices_array,
                          const std::vector<tTrianglePtr > &triangles_array, bool silent = false);
};