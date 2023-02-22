#include "AcousticManager.h"
#include "sim/AcousticBody.h"
#include "sim/KinematicBody.h"
#include "utils/FileUtil.h"
#include "utils/JsonUtil.h"
#include "utils/MathUtil.h"
#include "utils/json/json.h"
cAcousticManager::cAcousticManager(int id_)
    : cBaseObject(eObjectType::ACOUSTICMANAGER_TYPE, id_)
{
}

struct tMaterialBasedInfo
{
    tMaterialBasedInfo()
    {
        mMaterialName = "";
        mId2TypeStr.clear();
        mMaterialData.clear();
    }
    std::string mMaterialName;
    std::vector<std::string> mId2TypeStr;
    std::vector<tDataVec> mMaterialData;
};

tMaterialBasedInfo LoadMaterial(const Json::Value &material_json)
{
    tMaterialBasedInfo new_material_info;
    // get many types in a single material
    auto type_names = material_json.getMemberNames();
    int n_types = type_names.size();
    new_material_info.mId2TypeStr.resize(n_types);
    for (int t = 0; t < n_types; t++)
        new_material_info.mId2TypeStr[t] = type_names[t];
    new_material_info.mMaterialData.resize(n_types);

    printf("%d types in this material\n", n_types);
    for (int j = 0; j < n_types; j++)
    {
        Json::Value obj_lst_in_a_type =
            material_json[new_material_info.mId2TypeStr[j]];
        int n_objs = obj_lst_in_a_type.size();
        printf("type %d name %s, has %d objs\n", j, type_names[j].c_str(),
               n_objs);
        // In this material, in this type, we have many objects
        for (int k = 0; k < n_objs; k++)
        {
            Json::Value cur_obj = obj_lst_in_a_type[k];

            tAcousticIniData new_data;
            new_data.type_str = cJsonUtil::ParseAsString("type_str", cur_obj);
            new_data.type_id = cJsonUtil::ParseAsString("type_id", cur_obj);
            new_data.obj_id = cJsonUtil::ParseAsString("obj_id", cur_obj);
            new_data.ini_path = cJsonUtil::ParseAsString("ini", cur_obj);
            new_data.surface_mesh_path =
                cJsonUtil::ParseAsString("surface_mesh", cur_obj);

            new_material_info.mMaterialData[j].push_back(new_data);
        }
    }
    return new_material_info;
}
std::vector<tMaterialBasedInfo> CreateMaterialBasedInfo(const std::string &path)
{
    Json::Value root;
    SIM_ASSERT(cJsonUtil::LoadJson(path, root));
    std::vector<tMaterialBasedInfo> ret = {};
    auto materials = root.getMemberNames();
    printf("get %d materials\n", materials.size());
    for (int i = 0; i < materials.size(); i++)
    {

        std::string mat_str = materials[i];
        Json::Value material_type_lst = root[mat_str];
        printf("--- for material %d %s---\n", i, mat_str.c_str());
        tMaterialBasedInfo material_based_info =
            LoadMaterial(material_type_lst);
        material_based_info.mMaterialName = mat_str;

        ret.push_back(material_based_info);
    }
    return ret;
}

std::vector<tMaterialBasedInfo> gMatBasedInfo;
std::vector<tDataVec> CreateTypeBasedInfo(std::string sum_path)
{

    Json::Value sum_conf;
    SIM_ASSERT(cJsonUtil::LoadJson(sum_path, sum_conf));
    std::vector<tDataVec> data_vec = {};
    auto keys = sum_conf.getMemberNames();
    for (int i = 0; i < keys.size(); i++)
    {
        std::string name = keys[i];
        Json::Value data_lst = sum_conf[name];
        tDataVec cur_data;
        for (int j = 0; j < data_lst.size(); j++)
        {
            tAcousticIniData new_data;
            new_data.type_str =
                cJsonUtil::ParseAsString("type_str", data_lst[j]);
            new_data.type_id = cJsonUtil::ParseAsString("type_id", data_lst[j]);
            new_data.obj_id = cJsonUtil::ParseAsString("obj_id", data_lst[j]);
            new_data.ini_path = cJsonUtil::ParseAsString("ini", data_lst[j]);
            new_data.surface_mesh_path =
                cJsonUtil::ParseAsString("surface_mesh", data_lst[j]);
            cur_data.push_back(new_data);
        }
        data_vec.push_back(cur_data);
        printf("id %d name %s num of data %d\n", i, name.c_str(),
               cur_data.size());
    }
    return data_vec;
}
void cAcousticManager::Init(const Json::Value &conf)
{
    cBaseObject::Init(conf);
    std::string sum_path =
        cJsonUtil::ParseAsString("summary_material_based_json", conf);

    if (false == cFileUtil::ExistsFile(sum_path))
    {
        printf("sum_path json %s doesn't exist\n", sum_path.c_str());
    }
    // this->mDataArray = CreateTypeBasedInfo(sum_path);
    gMatBasedInfo = CreateMaterialBasedInfo(sum_path);
    {
        tVector3i res =
            cJsonUtil::ReadVectorJson("default_mat_type_data_id", conf, 3)
                .cast<int>();
        mCurSel.select_mat_id = res[0];
        mCurSel.select_type_id = res[1];
        mCurSel.select_data_id = res[2];
    }

    Reload();
}
#include "imgui.h"
bool gDrawTet = false;
void cAcousticManager::Update(_FLOAT dt) {}
void cAcousticManager::UpdateImGui()
{
    mAcousticBody->UpdateImGui();
    ImGui::Text("num of materials %d %s, cur type %d %s, num of data %d ",
                gMatBasedInfo.size(),
                gMatBasedInfo[mCurSel.select_mat_id].mMaterialName.c_str(),

                mCurSel.select_type_id,
                gMatBasedInfo[mCurSel.select_mat_id]
                    .mId2TypeStr[mCurSel.select_type_id]
                    .c_str(),
                gMatBasedInfo[mCurSel.select_mat_id]
                    .mMaterialData[mCurSel.select_type_id]
                    .size());

    auto cur_data =
        gMatBasedInfo[mCurSel.select_mat_id]
            .mMaterialData[mCurSel.select_type_id][mCurSel.select_data_id];

    ImGui::Text("ini path %s", cur_data.ini_path.c_str());
    ImGui::Text("mesh path %s", cur_data.surface_mesh_path.c_str());

    tSelInfo old_sel = mCurSel;
    ImGui::Checkbox("draw tet", &gDrawTet);
    // select obj type
    if (ImGui::BeginCombo(
            "select material",
            gMatBasedInfo[mCurSel.select_mat_id].mMaterialName.c_str()))
    {
        for (int i = 0; i < gMatBasedInfo.size(); i++)
        {
            auto cur_str = gMatBasedInfo[i].mMaterialName;
            bool is_selected = i == mCurSel.select_mat_id;

            if (ImGui::Selectable(cur_str.c_str(), is_selected))
            {
                mCurSel.select_mat_id = i;
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    Correct();
    // select data type

    const auto &cur_mat_info = gMatBasedInfo[mCurSel.select_mat_id];
    std::string cur_type_str = cur_mat_info.mId2TypeStr[mCurSel.select_type_id];
    if (ImGui::BeginCombo("select type", cur_type_str.c_str()))
    {
        for (int i = 0;
             i < gMatBasedInfo[mCurSel.select_mat_id].mId2TypeStr.size(); i++)
        {
            auto cur_str = gMatBasedInfo[mCurSel.select_mat_id].mId2TypeStr[i];
            bool is_selected = i == mCurSel.select_type_id;

            if (ImGui::Selectable(cur_str.c_str(), is_selected))
            {
                mCurSel.select_type_id = i;
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    Correct();
    if (ImGui::BeginCombo("select data id",
                          std::to_string(mCurSel.select_data_id).c_str()))
    {
        auto &cur_data = gMatBasedInfo[mCurSel.select_mat_id]
                             .mMaterialData[mCurSel.select_type_id];
        for (int i = 0; i < cur_data.size(); i++)
        {

            auto cur_str = cur_data[i].obj_id;
            bool is_selected = i == mCurSel.select_data_id;

            if (ImGui::Selectable(cur_str.c_str(), is_selected))
            {
                mCurSel.select_data_id = i;
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (old_sel.select_data_id != mCurSel.select_data_id ||
        old_sel.select_type_id != mCurSel.select_type_id ||
        old_sel.select_data_id != mCurSel.select_data_id)
    {
        Reload();
    }
}
#include "sim/KinematicBodyBuilder.h"
void cAcousticManager::Correct()
{

    mCurSel.select_mat_id =
        cMathUtil::Clamp(mCurSel.select_mat_id, 0, gMatBasedInfo.size() - 1);
    mCurSel.select_type_id =

        cMathUtil::Clamp(
            mCurSel.select_type_id, 0,
            gMatBasedInfo[mCurSel.select_mat_id].mId2TypeStr.size() - 1);

    mCurSel.select_data_id =
        cMathUtil::Clamp(mCurSel.select_data_id, 0,
                         gMatBasedInfo[mCurSel.select_mat_id]
                                 .mMaterialData[mCurSel.select_type_id]
                                 .size() -
                             1);
}
void cAcousticManager::Reload()
{
    Correct();
    auto cur_data =
        gMatBasedInfo[mCurSel.select_mat_id]
            .mMaterialData[mCurSel.select_type_id][mCurSel.select_data_id];
    std::string ini_path = cur_data.ini_path;
    Json::Value val;
    val["object_name"] = "acoustic_body";
    val["ini_path"] = ini_path;
    mAcousticBody = std::make_shared<cAcousticBody>(-1);
    mAcousticBody->Init(val);
    tVector3 aabb_min, aabb_max;
    mAcousticBody->CalcAABB(aabb_min, aabb_max);
    mAcousticBody->Shift(tVector3(0, -aabb_min[1] + 1e-2, 0));

    // load good trimesh

    printf("begin to load surfafce mesh %s\n",
           cur_data.surface_mesh_path.c_str());

    mSurf = std::dynamic_pointer_cast<cKinematicBody>(
        BuildKinematicBodyFromObjPath("surf", cur_data.surface_mesh_path, -1));
    mSurf->CalcAABB(aabb_min, aabb_max);
    mSurf->MoveTranslation(tVector3(0, -aabb_min[1] + 1e-2, 0));

    {
        printf("surf num of tris %d, tet num of tris %d\n",
               mSurf->GetTriangleArray().size(),
               mAcousticBody->GetTriangleArray().size());
    }

    mAcousticBody->UpdateRenderingResource();
    mSurf->UpdateRenderingResource(false);
}

void cAcousticManager::ApplyUserPerturbForceOnce(tPerturb *perb)
{
    mAcousticBody->ApplyUserPerturbForceOnce(perb);
}
void cAcousticManager::UpdateRenderingResource(
    bool enable_edge /*= enable_edge*/)
{
    // if (mAcousticBody != nullptr)
    // {
    //     mAcousticBody->UpdateRenderingResource();
    //     mSurf->UpdateRenderingResource(false);
    // }
}
#include "sim/KinematicBody.h"
std::vector<cRenderResourcePtr> cAcousticManager::GetRenderingResource() const
{
    if (mAcousticBody != nullptr)
    {
        if (gDrawTet == true)
        {
            return mAcousticBody->GetRenderingResource();
        }
        else
        {
            return mSurf->GetRenderingResource();
        }
    }
    return {};
}

const std::vector<tVertexPtr> &cAcousticManager::GetVertexArray() const
{
    return mAcousticBody->GetVertexArray();
}
const std::vector<tEdgePtr> &cAcousticManager::GetEdgeArray() const
{
    return mAcousticBody->GetEdgeArray();
}
const std::vector<tTrianglePtr> &cAcousticManager::GetTriangleArray() const
{
    return mAcousticBody->GetTriangleArray();
}

void cAcousticManager::ChangeTriangleColor(int tri_id, const tVector3 &color)
{
    if (gDrawTet == true)
    {
        mAcousticBody->ChangeTriangleColor(tri_id, color);
    }
    else
    {
        mSurf->ChangeTriangleColor(tri_id, color);
    }
}

const std::vector<tVertexPtr> &cAcousticManager::GetCollisionVertexArray() const
{
    return mAcousticBody->GetVertexArray();
}
const std::vector<tTrianglePtr> &
cAcousticManager::GetCollisionTriangleArray() const
{
    return mAcousticBody->GetTriangleArray();
}