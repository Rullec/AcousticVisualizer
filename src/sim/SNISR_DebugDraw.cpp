#include "SNISR_DebugDraw.h"
#include "utils/JsonUtil.h"
#include "utils/LogUtil.h"
#include "utils/json/json.h"

cSNISRDebugDrawBall::cSNISRDebugDrawBall(int obj_id)
    : cBaseObject(eObjectType::SNISR_DEBUG_DRAW_TYPE, obj_id)
{
}
void cSNISRDebugDrawBall::Init(const Json::Value &conf)
{
    cBaseObject::Init(conf);
    mPosInfoPath = cJsonUtil::ParseAsString("pos_info", conf);
    int num_limit = cJsonUtil::ParseAsInt("limit_point_num", conf);

    // parse pos info
    Json::Value val;
    printf("begin to load %s\n", mPosInfoPath.c_str());
    if (false == cJsonUtil::LoadJson(mPosInfoPath, val))
    {
        SIM_ERROR("fail to load pos info {}", mPosInfoPath);
        exit(1);
    }
    printf("loading done\n");

    // init bidirectional map: offset - label
    mOffset2LabelId.resize(val.getMemberNames().size());
    for (int offset = 0; offset < val.getMemberNames().size(); offset++)
    {
        int label_id = std::atoi(val.getMemberNames()[offset].c_str());
        mOffset2LabelId[offset] = label_id;
        mLabel2OffsetId[label_id] = offset;
    }

    mOffsetToPoints.resize(mOffset2LabelId.size());
    // init offset to points
    mOffset2LabelName.resize(mOffset2LabelId.size());
    for (int off = 0; off < mOffset2LabelId.size(); off++)
    {
        Json::Value pos_json = cJsonUtil::ParseAsValue(
            std::to_string(mOffset2LabelId[off]), val)["pos"];
        std::string name =
            cJsonUtil::ParseAsValue(std::to_string(mOffset2LabelId[off]),
                                    val)["cate_name"]
                .asString();
        mOffset2LabelName[off] = name;
        int num_of_pts = pos_json.size();
        printf("get %d points\n", num_of_pts);

        int sample_gap = 1;
        while (num_of_pts * 1.0 / sample_gap > num_limit)
        {
            sample_gap *= 2;
        }
        int new_pts = int(num_of_pts / sample_gap) + 1;
        printf("draw %d pts, sample gap %d\n", new_pts, sample_gap);
        tMatrixXf pts(3, new_pts);
        for (int j = 0; j < new_pts; j++)
        {
            int cur_idx = SIM_MIN(j * sample_gap, num_of_pts - 1);

            tVectorX vec = cJsonUtil::ReadVectorJson(pos_json[cur_idx]);
            pts.col(j) = vec.segment(0, 3).cast<float>();
        }

        mOffsetToPoints[off].push_back(pts);
    }
    mEnableDrawPts.resize(mOffset2LabelId.size());
    mEnableDrawPts.setOnes();

    // init offset to resources
    InitRenderResource();
}
std::vector<cRenderResourcePtr>
cSNISRDebugDrawBall::GetRenderingResource() const
{
    std::vector<cRenderResourcePtr> res = {};
    for (int off = 0; off < mOffset2LabelId.size(); off++)
    {
        if (mEnableDrawPts[off] == true)
        {
            res.insert(res.end(), mOffsetToRenderResource[off].begin(),
                       mOffsetToRenderResource[off].end());
        }
    }
    return res;
}
#include "imgui.h"
using tVector3f = Eigen::Vector3f;
void cSNISRDebugDrawBall::UpdateImGui()
{
    for (int off = 0; off < mOffset2LabelId.size(); off++)
    {
        std::string name =
            std::to_string(mOffset2LabelId[off]) + "_" + mOffset2LabelName[off];
        ImGui::Checkbox(name.c_str(), &mEnableDrawPts[off]);
    }
}
static std::vector<cBaseObjectPtr> obj_lst = {};
void cSNISRDebugDrawBall::UpdateRenderingResource(bool e) {}
#include "sim/KinematicBody.h"
#include "sim/KinematicBodyBuilder.h"
void cSNISRDebugDrawBall::InitRenderResource()
{
    mOffsetToRenderResource.resize(mOffset2LabelId.size());
    for (int off = 0; off < mOffset2LabelId.size(); off++)
    {
        tRenderResourcePerLabel resource(0);
        const tPtData &pt_data = mOffsetToPoints[off];

        for (auto &mat : pt_data)
        {
            for (int i = 0; i < mat.cols(); i++)
            {
                tVector3 tar_pos = mat.col(i).cast<_FLOAT>();

                std::string ball_path = "./data/sphere.obj";
                auto ball_obj =
                    BuildKinematicBodyFromObjPath("cons ball", ball_path, -1);
                cKinematicBodyPtr kin_obj =
                    std::dynamic_pointer_cast<cKinematicBody>(ball_obj);
                tVector3 aabb_min, aabb_max;
                kin_obj->CalcAABB(aabb_min, aabb_max);
                obj_lst.push_back(ball_obj);
                kin_obj->ApplyScale(1e-3);
                kin_obj->SetCurrentPos(tar_pos);

                kin_obj->CalcAABB(aabb_min, aabb_max);

                kin_obj->UpdateRenderingResource();
                auto res = kin_obj->GetRenderingResource();
                resource.insert(resource.end(), res.begin(), res.end());
            }
        }
        mOffsetToRenderResource[off] = resource;
        printf("label %d resource num %d\n", mOffset2LabelId[off],
               resource.size());
    }
}