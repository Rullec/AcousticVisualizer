#include "RenderUtil.h"
#include "geometries/Primitives.h"
#include "utils/ColorUtil.h"

tCPURenderBuffer::tCPURenderBuffer()
{
    mBuffer = nullptr;
    mNumOfEle = 0;
}
void cRenderUtil::CalcTriangleDrawBufferSingle(tVertexPtr v0, tVertexPtr v1,
                                               tVertexPtr v2,

                                               const tVector4 &color,
                                               Eigen::Map<tVectorXf> &buffer,
                                               int &st_pos)
{
    buffer.segment(st_pos, 3) = v0->mPos.segment(0, 3).cast<float>();
    buffer.segment(st_pos + 3, 4) = color.cast<float>();
    // buffer[st_pos + 6] = 0.5;
    buffer.segment(st_pos + 7, 3) = v0->mNormal.segment(0, 3).cast<float>();

    st_pos += RENDERING_SIZE_PER_VERTICE;
    buffer.segment(st_pos, 3) = v1->mPos.segment(0, 3).cast<float>();
    buffer.segment(st_pos + 3, 4) = color.cast<float>();
    // buffer[st_pos + 6] = 0.5;
    buffer.segment(st_pos + 7, 3) = v1->mNormal.segment(0, 3).cast<float>();

    st_pos += RENDERING_SIZE_PER_VERTICE;
    buffer.segment(st_pos, 3) = v2->mPos.segment(0, 3).cast<float>();
    buffer.segment(st_pos + 3, 4) = color.cast<float>();
    buffer.segment(st_pos + 7, 3) = v2->mNormal.segment(0, 3).cast<float>();
    // buffer[st_pos + 6] = 0.5;
    st_pos += RENDERING_SIZE_PER_VERTICE;
}

void cRenderUtil::CalcEdgeDrawBufferSingle(tVertexPtr v0, tVertexPtr v1,
                                           const tVector4 &edge_normal,
                                           Eigen::Map<tVectorXf> &buffer,
                                           int &st_pos, const tVector4 &color)
{

    tVector3 bias_amp = 1e-4f * edge_normal.segment(0, 3); // 0.1 mm

    // pos, color, normal
    buffer.segment(st_pos, 3) =
        (v0->mPos.segment(0, 3) + bias_amp).cast<float>();
    buffer.segment(st_pos + 3, 4) = color.cast<float>();
    buffer.segment(st_pos + 7, 3) = tVector3(0, 0, 0).cast<float>();
    st_pos += RENDERING_SIZE_PER_VERTICE;

    buffer.segment(st_pos, 3) =
        (v1->mPos.segment(0, 3) + bias_amp).cast<float>();
    buffer.segment(st_pos + 3, 4) = color.cast<float>();
    buffer.segment(st_pos + 7, 3) = tVector3(0, 0, 0).cast<float>();
    st_pos += RENDERING_SIZE_PER_VERTICE;
}

void cRenderUtil::CalcPointDrawBufferSingle(const tVector4 &v_pos,
                                            const tVector4 &v_color,
                                            Eigen::Map<tVectorXf> &buffer,
                                            int &st_pos)
{
    buffer.segment(st_pos, 3) = v_pos.segment(0, 3).cast<float>();
    buffer.segment(st_pos + 3, 4) = v_color.cast<float>();
    st_pos += RENDERING_SIZE_PER_VERTICE;
}

static tVectorXf gAxesData;
cRenderResourcePtr cRenderUtil::GetAxesRenderingResource()
{
    auto res = std::make_shared<cRenderResource>();
    res->mName = "axes";
    gAxesData.noalias() = tVectorXf::Zero(6 * RENDERING_SIZE_PER_VERTICE);

    int idx = 0;
    for (int i = 0; i < 3; i++)
    {
        /*
        1. pos R3
        2. color R4 (Red, Blue, Green)
        3. normal R3 (zero vec)
        4. uv R2 (nan)
        */

        tVector3 st_pos = tVector3::Zero();
        tVector3 end_pos = tVector3::Zero();
        end_pos[i] = 100.0;

        tVector4 color = tVector4::Zero();
        color[3] = 1.0;
        color[i] = 1.0;

        tVector2 uv = tVector2::Ones() * std::nanf("");

        // start point

        for (int j = 0; j < 2; j++)
        {
            // pos
            gAxesData.segment(idx, 3) =
                (j == 0 ? st_pos : end_pos).cast<float>();
            idx += 3;
            // color
            gAxesData.segment(idx, 4) = color.cast<float>();
            idx += 4;
            // normal
            gAxesData.segment(idx, 3).setZero();
            idx += 3;
            // uv
            gAxesData.segment(idx, 2) = uv.cast<float>();
            idx += 2;
        }
        // end point
    }
    res->mLineBuffer.mNumOfEle = gAxesData.size();
    res->mLineBuffer.mBuffer = gAxesData.data();
    return res;
}