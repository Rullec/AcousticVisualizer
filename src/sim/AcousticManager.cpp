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

void cAcousticManager::Init(const Json::Value &conf)
{
    cBaseObject::Init(conf);
    std::string sum_path = cJsonUtil::ParseAsString("summary_json", conf);

    if (false == cFileUtil::ExistsFile(sum_path))
    {
        printf("sum_path json %s doesn't exist\n", sum_path.c_str());
    }
    Json::Value sum_conf;
    SIM_ASSERT(cJsonUtil::LoadJson(sum_path, sum_conf));

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
        mDataArray.push_back(cur_data);
        printf("id %d name %s num of data %d\n", i, name.c_str(),
               cur_data.size());
    }

    {
        tVector2i res =
            cJsonUtil::ReadVectorJson("default_type_and_data_id", conf, 2)
                .cast<int>();

        mCurSel.select_type_id =
            cMathUtil::Clamp(res[0], 0, this->mDataArray.size() - 1);
        mCurSel.select_data_id = cMathUtil::Clamp(
            res[1], 0, mDataArray[mCurSel.select_type_id].size() - 1);
    }

    Reload();
}
#include "imgui.h"
bool gDrawTet = false;
void cAcousticManager::Update(_FLOAT dt) {}
void cAcousticManager::UpdateImGui()
{
    ImGui::Text("num of types %d, cur type %d %s, num of data %d ",
                mDataArray.size(), mCurSel.select_data_id,
                mDataArray[mCurSel.select_data_id][0].type_str.c_str(),
                mDataArray[mCurSel.select_data_id].size());

    auto cur_data = mDataArray[mCurSel.select_type_id][mCurSel.select_data_id];
    ImGui::Text("ini path %s", cur_data.ini_path.c_str());
    ImGui::Text("mesh path %s", cur_data.surface_mesh_path.c_str());

    tSelInfo old_sel = mCurSel;
    ImGui::Checkbox("draw tet", &gDrawTet);
    // select obj type
    if (ImGui::BeginCombo(
            "select obj type",
            mDataArray[mCurSel.select_type_id][0].type_str.c_str()))
    {
        for (int i = 0; i < mDataArray.size(); i++)
        {
            auto cur_str = mDataArray[i][0].type_str;
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

    // select data type
    if (ImGui::BeginCombo("select data id",
                          std::to_string(mCurSel.select_data_id).c_str()))
    {
        for (int i = 0; i < mDataArray[mCurSel.select_type_id].size(); i++)
        {
            auto cur_str = mDataArray[mCurSel.select_type_id][i].obj_id;
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
        old_sel.select_type_id != mCurSel.select_type_id)
    {
        Reload();
    }
}
#include "sim/KinematicBodyBuilder.h"
void cAcousticManager::Reload()
{
    auto cur_data = mDataArray[mCurSel.select_type_id][mCurSel.select_data_id];
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