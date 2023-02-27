#include "ObjUtil.h"
#include "utils/RotUtil.h"
#include <set>
#define TINYOBJLOADER_IMPLEMENTATION
#include "geometries/Primitives.h"
#include "geometries/Triangulator.h"
#include "tinyobjloader/tiny_obj_loader.h"
#include "utils/FileUtil.h"
#include "utils/LogUtil.h"
#include "utils/RenderUtil.h"
#include <iostream>
typedef std::tuple<int, int, int> tTriVid;
auto make_unorder_trivid = [](int a, int b, int c) -> tTriVid
{
    std::vector<int> res = {a, b, c};
    std::sort(res.begin(), res.end());
    return tTriVid(res[0], res[1], res[2]);
};

auto make_ordered_trivid = [](int a, int b, int c) -> tTriVid
{
    tVector3i res(a, b, c);
    int min_idx = 0;
    while (res[min_idx] != res.minCoeff())
    {
        min_idx += 1;
    }
    switch (min_idx)
    {
    case 0:
        break;
    case 1:
        res = tVector3i(b, c, a);
        break;
    case 2:
        res = tVector3i(c, a, b);
        break;
    default:
        break;
    }

    return tTriVid(res[0], res[1], res[2]);
};

std::string GetTexturePath(std::string tex_img_path, std::string obj_path)
{
    if (tex_img_path.size() == 0)
    {
        return "";
    }
    // 1. global path or relative path w.r.t cwd

    bool found_tex = false;
    std::string final_path = "";
    if (cFileUtil::ExistsFile(tex_img_path))
    {
        final_path = tex_img_path;
        found_tex = true;
    }
    else
    {
        std::string obj_rel_path = cFileUtil::ConcatFilename(
            cFileUtil::GetDir(obj_path), tex_img_path);
        printf("[obj_read] try possible path %s\n", obj_rel_path.c_str());
        if (cFileUtil::ExistsFile(obj_rel_path))
        {
            final_path = obj_rel_path;
            found_tex = true;
        }
        // relative
        // 1. working directory 2. obj file's directory
    }
    if (found_tex)
    {
        printf("found tex for %s: %s\n", tex_img_path.c_str(),
               final_path.c_str());
    }
    else
    {
        SIM_ERROR("not found tex for {}", tex_img_path);
    }
    return final_path;
}
std::vector<tMeshMaterialInfoPtr>
LoadMaterials(std::string obj_path,
              const std::vector<tinyobj::material_t> &materials)
{

    std::vector<tMeshMaterialInfoPtr> array(0);
    for (int i = 0; i < materials.size(); i++)
    {
        const auto &mat = materials[i];
        /*
            ambient color   // Ka
            diffuse color   // Kd
            specular color  // Ks
            Ns ; 96
        */

        auto new_mat = std::make_shared<tMeshMaterialInfo>();
        new_mat->Ns = mat.shininess;
        new_mat->mName = mat.name;
        new_mat->Ka =
            Eigen::Vector3f(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
        new_mat->Kd =
            Eigen::Vector3f(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        new_mat->Ks =
            Eigen::Vector3f(mat.specular[0], mat.specular[1], mat.specular[2]);
        new_mat->mTexImgPath = GetTexturePath(mat.diffuse_texname, obj_path);
        new_mat->mEnableTexutre = new_mat->mTexImgPath.size() != 0;
        SIM_INFO("load material {} {}, Ka {}, Kd {}, Ks {}, Ns {}, tex {} "
                 "enable tex {}",
                 new_mat->mName.c_str(), i, new_mat->Ka.transpose(),
                 new_mat->Kd.transpose(), new_mat->Ks.transpose(), new_mat->Ns,
                 new_mat->mTexImgPath, new_mat->mEnableTexutre);
        array.push_back(new_mat);
    }

    return array;
}

void LoadVertices(const tinyobj::attrib_t &attribs,
                  std::vector<tVertexPtr> &v_array)
{
    int num_of_v = attribs.vertices.size() / 3;
    v_array.resize(num_of_v, nullptr);
    for (int i = 0; i < num_of_v; i++)
    {
        v_array[i] = std::make_shared<tVertex>();
        v_array[i]->mPos =
            tVector4(attribs.vertices[3 * i + 0], attribs.vertices[3 * i + 1],
                     attribs.vertices[3 * i + 2], 1);
        // printf("v %d %.3f %.3f %.3f\n", i, v_array[i]->mPos[0],
        //        v_array[i]->mPos[1], v_array[i]->mPos[2]);
    }
    printf("load %d vertices\n", num_of_v);
}
template <class dtype> void VertexResort(dtype &a, dtype &b, dtype &c)
{
    tVector3i res(a, b, c);
    int min_id = 0;
    while (res.minCoeff() != res[min_id])
        min_id += 1;
    switch (min_id)
    {
    case 0:
        break;
    case 1:
        res = tVector3i(b, c, a);
        break;
    case 2:
        break;
        res = tVector3i(c, a, b);
    default:
        SIM_ERROR("invalid {}", min_id);
    }
    a = res[0];
    b = res[1];
    c = res[2];
}

void LoadTriangles(const std::vector<tinyobj::shape_t> &shapes,
                   const tinyobj::attrib_t &attribs,
                   std::vector<tVertexPtr> &v_array,
                   std::vector<tTrianglePtr> &t_array,
                   std::vector<tMeshMaterialInfoPtr> &mat_info)
{
    t_array.clear();
    printf("total shapes %d\n", shapes.size());
    for (int s = 0; s < shapes.size(); s++)
    {
        // printf("---------- load shape %d / %d ------------\n", s,
            //    shapes.size());
        const auto &cur_shape = shapes[s];
        const auto &cur_indices = cur_shape.mesh.indices;

        // Loop over faces(polygon)
        size_t index_offset = 0;
        int num_of_faces_shape = cur_shape.mesh.num_face_vertices.size();
        // printf("num of faces %d\n", num_of_faces_shape);
        // SIM_ASSERT(cur_shape.mesh.num_face_vertices.size() == 3);
        for (size_t f = 0; f < num_of_faces_shape; f++)
        {
            // for this triangle
            size_t fv = size_t(cur_shape.mesh.num_face_vertices[f]);
            SIM_ASSERT(fv == 3);
            tTrianglePtr t = std::make_shared<tTriangle>();
            t->mId0 = cur_indices[index_offset + 0].vertex_index;
            t->mId1 = cur_indices[index_offset + 1].vertex_index;
            t->mId2 = cur_indices[index_offset + 2].vertex_index;
            // VertexResort(t->mId0, t->mId1, t->mId2);, this exchaning may
            // cause the texture order get's wrong (below)

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++)
            {
                // access to vertex
                tinyobj::index_t idx = cur_indices[index_offset + v];
                tinyobj::real_t vx =
                    attribs.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy =
                    attribs.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz =
                    attribs.vertices[3 * size_t(idx.vertex_index) + 2];
                int vertex_id = idx.vertex_index;

                // = no normal data
                if (idx.normal_index >= 0)
                {
                    tinyobj::real_t nx =
                        attribs.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny =
                        attribs.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz =
                        attribs.normals[3 * size_t(idx.normal_index) + 2];
                    v_array[vertex_id]->mNormal =
                        tVector4(nx, ny, nz, 0).normalized();
                }

                // // Check if `texcoord_index` is zero or positive.
                // negative = no texcoord data
                if (idx.texcoord_index >= 0)
                {
                    tinyobj::real_t tx =
                        attribs.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty =
                        attribs.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    t->muv[v] = tVector2(tx, ty);
                }
            }
            index_offset += fv;

            // per-face material
            uint mat_id = cur_shape.mesh.material_ids[f];
            int tid = t_array.size();

            if (mat_id < 0 || mat_id >= mat_info.size())
            {
                // printf("face %d has no material, set to default material
                // 0\n",
                //        tid);
                mat_id = 0;
            }
            mat_info[mat_id]->mTriIdArray.push_back(tid);
            t_array.push_back(t);
        }
    }
}

int GetNewTriIdAfterDelete(const std::vector<int> &removed_tid, int old_tid)
{
    int new_tid = old_tid;
    for (auto &x : removed_tid)
    {
        if (x < old_tid)
            new_tid--;
    }
    return new_tid;
}
template <class T>
inline void erase_selected(std::vector<T> &v, const std::vector<int> &selection)
{
    v.resize(std::distance(
        v.begin(),
        std::stable_partition(
            v.begin(), v.end(),
            [&selection, &v](const T &item)
            {
                return !std::binary_search(
                    selection.begin(), selection.end(),
                    static_cast<int>(static_cast<const T *>(&item) - &v[0]));
            })));
}

void RemoveDuplicateTriangles(std::vector<tTrianglePtr> &t_array,
                              std::vector<tMeshMaterialInfoPtr> &mat_info)
{
    printf("begin to remove duplicate triangles\n");
    std::set<tTriVid> set_vids;
    auto it = t_array.begin();
    std::vector<int> deleted_tri = {};
    while (it != t_array.end())
    {

        auto cur_vid =
            make_ordered_trivid((*it)->mId0, (*it)->mId1, (*it)->mId2);
        auto find_res = set_vids.find(cur_vid);
        if (find_res == set_vids.end())
        {
            // not found
            set_vids.insert(cur_vid);
        }
        else
        {
            // delete this t
            // it = t_array.erase(it);
            deleted_tri.push_back(it - t_array.begin());
        }
        it += 1;
    }
    erase_selected(t_array, deleted_tri);
    printf("update tid in materials\n");
    for (auto &mat : mat_info)
    {
        for (auto &id : mat->mTriIdArray)
        {
            id = GetNewTriIdAfterDelete(deleted_tri, id);
        }
    }
    SIM_WARN("remove duplicate tri {}, left tri {}", deleted_tri.size(),
             t_array.size());
    // if (num_of_duplicate_tri != 0)
    // {
    //     SIM_ERROR("Please handle the material tri id!\n");
    //     exit(1);
    // }
}
std::vector<int>
GetMapFromTidToMatId(int num_of_t, std::vector<tMeshMaterialInfoPtr> &mat_info)
{
    std::vector<int> new_map(num_of_t, -1);
    for (int mid = 0; mid < mat_info.size(); mid++)
    {
        auto x = mat_info[mid];
        for (auto &tid : x->mTriIdArray)
        {
            new_map[tid] = mid;
        }
    }
    return new_map;
}

void HandleTwoSideTri(std::vector<tVertexPtr> &v_array,
                      std::vector<tTrianglePtr> &t_array)
{

    int old_num_of_v = v_array.size();
    std::vector<tVertexPtr> new_v_array = {};
    std::set<tTriVid> set_vids;
    int num_of_two_faces = 0;
    std::map<int, int> old_vid_to_new_vid = {};
    std::vector<int> duplicate_tid = {};
    for (int i = 0; i < t_array.size(); i++)
    {
        auto cur_vid = make_unorder_trivid(t_array[i]->mId0, t_array[i]->mId1,
                                           t_array[i]->mId2);

        if (set_vids.find(cur_vid) == set_vids.end())
        {
            // no double face
            set_vids.insert(cur_vid);
        }
        else
        {
            duplicate_tid.push_back(i);

            int wait_to_create_vid[3] = {
                t_array[i]->mId0,
                t_array[i]->mId1,
                t_array[i]->mId2,
            };
            int new_vid[3] = {-1, -1, -1};
            for (int j = 0; j < 3; j++)
            {
                int old_vid = wait_to_create_vid[j];
                if (old_vid_to_new_vid.end() ==
                    old_vid_to_new_vid.find(old_vid))
                {
                    // create new one
                    new_v_array.push_back(
                        std::make_shared<tVertex>(*(v_array[old_vid])));
                    new_vid[j] = new_v_array.size() + old_num_of_v - 1;
                }
                else
                {
                    new_vid[j] = old_vid_to_new_vid.find(old_vid)->second;
                }
            }

            t_array[i]->mId0 = new_vid[0];
            t_array[i]->mId1 = new_vid[1];
            t_array[i]->mId2 = new_vid[2];
            num_of_two_faces += 1;
        }
    }

    v_array.insert(v_array.end(), new_v_array.begin(), new_v_array.end());
    printf("[log] found %d double faces, add %d new vertices\n",
           num_of_two_faces, new_v_array.size());
    float eps = 1e-5;
    for (auto dup_tid : duplicate_tid)
    {
        int vid[3] = {t_array[dup_tid]->mId0, t_array[dup_tid]->mId1,
                      t_array[dup_tid]->mId2};
        tVector4 normal =
            (v_array[vid[1]]->mPos - v_array[vid[0]]->mPos)
                .cross3(v_array[vid[2]]->mPos - v_array[vid[1]]->mPos);
        normal[3] = 0;
        normal.normalized();
        for (int j = 0; j < 3; j++)
        {
            v_array[vid[j]]->mPos += normal * eps;
            // printf("v %d shift move %.2e %.2e %.2e\n", vid[j],
            //        (normal * eps)[0], (normal * eps)[1], (normal * eps)[2]);
        }
    }
}
void cObjUtil::LoadObj(const std::string &path,
                       std::vector<tVertexPtr> &v_array,
                       std::vector<tEdgePtr> &e_array,
                       std::vector<tTrianglePtr> &t_array,
                       std::vector<tMeshMaterialInfoPtr> &mat_info)
{
    printf("begin to load obj %s\n", path.c_str());
    v_array.clear();
    e_array.clear();
    t_array.clear();

    // example code from https://github.com/tinyobjloader/tinyobjloader
    std::string inputfile = path;
    tinyobj::ObjReaderConfig reader_config;
    std::string dir = cFileUtil::GetDir(path);
    reader_config.mtl_search_path = dir; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config))
    {
        if (!reader.Error().empty())
        {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty())
    {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto &attrib = reader.GetAttrib();
    auto &shapes = reader.GetShapes();
    auto &materials = reader.GetMaterials();

    mat_info = LoadMaterials(path, materials);
    if (mat_info.size() == 0)
    {
        printf("no material in obj, set to default material\n");
        mat_info.push_back(tMeshMaterialInfo::GetDefaultMaterialInfo());
    }
    LoadVertices(attrib, v_array);
    LoadTriangles(shapes, attrib, v_array, t_array, mat_info);
    RemoveDuplicateTriangles(t_array, mat_info);
    HandleTwoSideTri(v_array, t_array);

    // for (int i = 0; i < v_array.size(); i++)
    // {
    //     printf("v%d : %.3f %.3f %.3f, uv %.2f %.2f\n", i,
    //     v_array[i]->mPos[0],
    //            v_array[i]->mPos[1], v_array[i]->mPos[2],
    //            v_array[i]->muv_simple[0], v_array[i]->muv_simple[1]);
    // }
    std::vector<int> tid2mid = GetMapFromTidToMatId(t_array.size(), mat_info);
    for (int tid = 0; tid < t_array.size(); tid++)
    {
        auto t = t_array[tid];
        // printf("tri %d v %d %d %d, mat id %d\n", tid, t->mId0, t->mId1,
        // t->mId2,
        //        tid2mid[tid]);
    }
    for (int i = 0; i < v_array.size(); i++)
    {
        if (v_array[i] == nullptr)
        {
            SIM_ERROR("vertex {} is empty, exit", i);
        }
    }
    std::vector<int> used_triangles(t_array.size(), 0);

    for (int i = 0; i < mat_info.size(); i++)
    {
        auto &mat = mat_info[i];
        for (auto &tid : mat->mTriIdArray)
        {
            used_triangles[tid] = 1;
        }
        // printf("material %d has %d tris\n", i, mat->mTriIdArray.size());
        // for (auto &t : mat->mTriIdArray)
        // {
        //     std::cout << t << " ";
        // }
        // std::cout << std::endl;
    }
    std::vector<int> empty_tids = {};
    for (int i = 0; i < t_array.size(); i++)
    {
        if (used_triangles[i] == 0)
        {
            empty_tids.push_back(i);
        }
    }
    printf("there are %d tris with no material, cannot be drawn\n",
           empty_tids.size());
    // HandleDoubleFace(v_array, t_array);
    cObjUtil::BuildEdge(v_array, e_array, t_array);
}
/**
 * \brief       Given vertex array and triangle array, build the edge list
 */

typedef std::pair<int, int> int_pair;
void cObjUtil::BuildEdge(const std::vector<tVertexPtr> &v_array,
                         std::vector<tEdgePtr> &e_array,
                         const std::vector<tTrianglePtr> &t_array)
{
    e_array.clear();

    // 1. build duplicate edge array
    // edge_id_pair -> triangle_id_pair
    std::map<int_pair, int_pair> edge_info;
    edge_info.clear();

    // for each triangle
    for (int t_id = 0; t_id < t_array.size(); t_id++)
    {
        tTrianglePtr tri = t_array[t_id];

        // check three edges
        for (int i = 0; i < 3; i++)
        {
            // for each edge
            int id0 = (i == 0) ? (tri->mId0)
                               : ((i == 1) ? tri->mId1 : (tri->mId2)),
                id1 = (i == 0) ? (tri->mId1)
                               : ((i == 1) ? tri->mId2 : (tri->mId0));

            if (id0 > id1)
            {
                std::swap(id0, id1);
            }
            auto edge_id_pairs = int_pair(id0, id1);
            std::map<int_pair, int_pair>::iterator it =
                edge_info.find(edge_id_pairs);

            // create new edge
            if (it == edge_info.end())
            {
                edge_info[edge_id_pairs] = int_pair(t_id, -1);
            }
            else
            {
                // use old edge
                auto &triangle_pair = it->second;
                if (false ==
                    (triangle_pair.first != -1 && triangle_pair.second == -1))
                {
                    printf(
                        "[error] the connected triangle of edge from v%d to "
                        "v%d are "
                        "t%d and t%d; but now t%d is also along these two v\n",
                        it->first.first, it->first.second, triangle_pair.first,
                        triangle_pair.second, t_id);

                    // exit(1);
                }
                triangle_pair.second = t_id;
                // it->second = t_id;

                // SIM_ASSERT(it->second.first != -1 && it->second.second ==
                // -1); it->second.second = t_id; it->second = t_id;
            }
        }
    }

    // set dataset for edges
    std::map<int_pair, int> edge_info_2_edge_id = {};
    int t_id = 0;
    for (auto t = edge_info.begin(); t != edge_info.end(); t++, t_id++)
    {
        int v0 = t->first.first, v1 = t->first.second;
        // SIM_ASSERT(v0 < v1);
        int tid0 = t->second.first, tid1 = t->second.second;
        tEdgePtr edge = std::make_shared<tEdge>();
        edge->mId0 = v0;
        edge->mId1 = v1;
        edge->mRawLength = (v_array[v0]->mPos - v_array[v1]->mPos).norm();
        edge->mTriangleId0 = tid0;
        edge->mTriangleId1 = tid1;
        edge->mIsBoundary = (tid1 == -1);
        e_array.push_back(edge);
        edge_info_2_edge_id[int_pair(v0, v1)] = t_id;
        // printf("[debug] edge %d, v0 %d, v1 %d, raw length %.3f, t0 %d, t1 %d,
        // is_boud %d\n",

        //    e_array.size() - 1, v0, v1, edge->mRawLength, tid0, tid1,
        //    edge->mIsBoundary);
    }

    for (int t = 0; t < t_array.size(); t++)
    {
        int v_id[3] = {t_array[t]->mId0, t_array[t]->mId1, t_array[t]->mId2};
        t_array[t]->mEId0 = edge_info_2_edge_id[int_pair(
            SIM_MIN(v_id[0], v_id[1]), SIM_MAX(v_id[0], v_id[1]))];
        t_array[t]->mEId1 = edge_info_2_edge_id[int_pair(
            SIM_MIN(v_id[1], v_id[2]), SIM_MAX(v_id[1], v_id[2]))];
        t_array[t]->mEId2 = edge_info_2_edge_id[int_pair(
            SIM_MIN(v_id[2], v_id[0]), SIM_MAX(v_id[2], v_id[0]))];
    }
    // std::cout << "[debug] build " << e_array.size() << " edges\n";
}

/**
 * \brief           Build plane geometry data
 */
void cObjUtil::BuildPlaneGeometryData(const _FLOAT scale,
                                      const tVector4 &plane_equation,
                                      std::vector<tVertexPtr> &vertex_array,
                                      std::vector<tEdgePtr> &edge_array,
                                      std::vector<tTrianglePtr> &triangle_array)
{
    vertex_array.clear();
    edge_array.clear();
    triangle_array.clear();
    // 1. calculate a general vertex array
    tVector4 cur_normal = tVector4(0, 1, 0, 0);
    tEigenArr<tVector4> pos_lst = {tVector4(1, 0, -1, 1),
                                   tVector4(-1, 0, -1, 1),
                                   tVector4(-1, 0, 1, 1), tVector4(1, 0, 1, 1)};
    tEigenArr<tVector3i> triangle_idx_lst = {tVector3i(0, 1, 3),
                                             tVector3i(3, 1, 2)};

    // build vertices
    for (auto &x : pos_lst)
    {
        tVertexPtr v = std::make_shared<tVertex>();
        v->mPos.noalias() = x;
        v->mPos.segment(0, 3) *= scale;
        vertex_array.push_back(v);
    }

    // build triangles
    for (auto &x : triangle_idx_lst)
    {
        tTrianglePtr tri = std::make_shared<tTriangle>();
        tri->mId0 = x[0];
        tri->mId1 = x[1];
        tri->mId2 = x[2];
        triangle_array.push_back(tri);
    }

    cObjUtil::BuildEdge(vertex_array, edge_array, triangle_array);
    cTriangulator::ValidateGeometry(vertex_array, edge_array, triangle_array);

    // rotation

    tVector4 normal = cMathUtil::CalcNormalFromPlane(plane_equation);
    tMatrix4 transform = cRotUtil::AxisAngleToRotmat(
        cMathUtil::CalcAxisAngleFromOneVectorToAnother(cur_normal, normal));

    // translation
    {
        tVector4 new_pt = transform * vertex_array[0]->mPos;
        tVector3 abc = plane_equation.segment(0, 3);
        _FLOAT k = (-plane_equation[3] - abc.dot(new_pt.segment(0, 3))) /
                   (abc.dot(normal.segment(0, 3)));
        transform.block(0, 3, 3, 1) = k * normal.segment(0, 3);
    }

    for (auto &x : vertex_array)
    {
        // std::cout << "old pos0 = " << x->mPos.transpose() << std::endl;
        x->mPos = transform * x->mPos;
        // std::cout << "eval = " << cMathUtil::EvaluatePlane(plane_equation,
        // x->mPos) << std::endl;
    }
    // exit(0);
}

// void cObjUtil::HandleDoubleFace(std::vector<tVertexPtr> &v_array,
//                                 std::vector<tTrianglePtr> &t_array)
// {

//     // 1. detect two-side triangles

//     for (auto &tri : t_array)
//     {
//         auto cur_tri_vid = make_trivid(tri->mId0, tri->mId1, tri->mId2);
//         if (tri_vid_set.find(cur_tri_vid) != tri_vid_set.end())
//         {
//             duplicate_tri.push_back(tri);
//         }
//         else
//         {
//             tri_vid_set.insert(cur_tri_vid);
//         }
//     }
//     SIM_INFO("get {} duplicated triangles (two-side)", duplicate_tri.size());

//     // 2. copy vertex
//     std::map<uint, uint> old_vid_to_new_vid;
//     for (auto &t : duplicate_tri)
//     {
//         uint vid[3] = {t->mId0, t->mId1, t->mId2};
//         for (int i = 0; i < 3; i++)
//         {
//             uint cur_vid = vid[i];
//             if (old_vid_to_new_vid.find(cur_vid) == old_vid_to_new_vid.end())
//             {
//                 // copy
//                 v_array.push_back(
//                     std::make_shared<tVertex>(*(v_array[cur_vid])));
//                 int new_vid = v_array.size() - 1;
//                 old_vid_to_new_vid[cur_vid] = new_vid;
//                 // insert into the result
//             }
//             int new_vid = old_vid_to_new_vid[cur_vid];
//             switch (i)
//             {
//             case 0:
//                 t->mId0 = new_vid;
//                 break;
//             case 1:
//                 t->mId1 = new_vid;
//                 break;
//             case 2:
//                 t->mId2 = new_vid;
//                 break;
//             default:
//                 break;
//             }
//         }
//     }
//     SIM_INFO("create new vertex {}", old_vid_to_new_vid.size());
// }