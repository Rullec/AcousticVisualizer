#include "AcousticBody.h"
#include "AudioOutput.h"
#include "INIReader.h"
#include "cameras/CameraBase.h"
#include "cameras/CameraBuilder.h"
#include "geometries/ObjUtil.h"
#include "geometries/Primitives.h"
#include "utils/ColorUtil.h"
#include "utils/FileUtil.h"
#include "utils/JsonUtil.h"
#include "utils/LogUtil.h"

const int SR = 44100; // 44100 HZ
tAcousticMaterialProp::tAcousticMaterialProp()
{
    mRho = -1;
    mA = -1;
    mB = -1;
}

tClickInfo::tClickInfo()
{
    mTriIds.clear();
    mAmp.clear();
    mNormal.clear();
    mTS = -1;
    mAudioAmp = -1;
}

cAcousticBody::cAcousticBody(int id)
    : cBaseObject(eObjectType::ACOUSTICBODY_TYPE, id)
{
    mEnableRangeClick = true;
    mEnableAudioScale = true;
    mRangeClickRadius = 1 * 1e-3;
    mCustomDampVectorScale = 1;
}
cAcousticBody::~cAcousticBody() {}

void cAcousticBody::InitFromIni(std::string ini_path, bool enable_start_sound)
{
    mIniPath = ini_path;
    if (cFileUtil::ExistsFile(mIniPath) == false)
    {
        SIM_ERROR("ini %s doesn't exist", mIniPath.c_str());
    }
    printf("load ini %s\n", mIniPath.c_str());

    // 2. load ini path
    INIReader reader(mIniPath);

    if (reader.ParseError() != 0)
    {
        SIM_ERROR("cannot parse ini %s", mIniPath);
    }

    mSurfaceObjPath = reader.Get("mesh", "surface_mesh", "INVALID_SURFACE_OBJ");

    mClickInfo.mTS = reader.GetFloat("audio", "TS", -1);
    mClickInfo.mAudioAmp = reader.GetFloat("audio", "amplitude", -1);

    mMomentsPath = reader.Get("transfer", "moments", "INVALID_MOMENT");

    mEigenPath = reader.Get("modal", "shape", "INVALID_EV");

    mAcousticProp.mRho = reader.GetFloat("modal", "density", -1);
    mAcousticProp.mA = reader.GetFloat("modal", "alpha", -1);
    mAcousticProp.mB = reader.GetFloat("modal", "beta", -1);

    mVmapPath = reader.Get("modal", "vtx_map", "INVALID_VMAP");

    mClickInfo.mTriIds = {
        static_cast<int>(reader.GetInteger("collisions", "ID", -1))};

    mClickInfo.mAmp = {reader.GetFloat("collisions", "amplitude", -1)};

    ;
    mClickInfo.mNormal = {tVector3(reader.GetFloat("collisions", "norm1", -1),
                                   reader.GetFloat("collisions", "norm2", -1),
                                   reader.GetFloat("collisions", "norm3", -1))};

    mCamPos = tVector3(reader.GetFloat("camera", "x", -1),
                       reader.GetFloat("camera", "y", -1),
                       reader.GetFloat("camera", "z", -1));

    cObjUtil::LoadObj(mSurfaceObjPath, mVertexArray, mEdgeArray, mTriangleArray,
                      mMatInfoArray);
    mTrianglesCOM.resize(mTriangleArray.size());
    for (int i = 0; i < GetNumOfTriangles(); i++)
    {
        mTrianglesCOM[i] = (mVertexArray[mTriangleArray[i]->mId0]->mPos +
                            mVertexArray[mTriangleArray[i]->mId1]->mPos +
                            mVertexArray[mTriangleArray[i]->mId2]->mPos)
                               .segment(0, 3) /
                           3;
    }
    // add color
    for (auto &v : mVertexArray)
    {
        v->mColor = ColorAn;
    }
    for (auto &l : mEdgeArray)
    {
        l->mColor = ColorBlack;
    }
    for (auto &l : mTriangleArray)
    {
        l->mColor = ColorBlue;
    }
    CalcTriangleInitArea();
    UpdateTriangleNormal();
    UpdateVertexNormalFromTriangleNormal();
    InitAudioGeo();
    InitAudioBuffer();

    if (enable_start_sound == true)
        AudioSynthesis();
}

void cAcousticBody::Init(const Json::Value &conf)
{
    cBaseObject::Init(conf);
    // 1. get ini path
    mIniPath = cJsonUtil::ParseAsString("ini_path", conf);
    InitFromIni(mIniPath, true);
}
#include "cameras/CameraFactory.h"
void cAcousticBody::Update(_FLOAT dt)
{
    mCamPos = cCameraFactory::getInstance()->GetCameraPos();
}

void load_vertex_map(const std::string &filename, int &num_v_fixed,
                     std::vector<int> &vmap_surf2tet)
{
    int id1, id2;

    std::ifstream fin(filename.c_str());
    if (fin.fail())
    {
        SIM_ERROR("Cannot read file: %s\n", filename.c_str());
        exit(2);
    }
    fin >> num_v_fixed >> id1; // # of fixed vertices in tet mesh
                               // & total number of surface vertices
    SIM_INFO("  # of fixed vertices: %d\n", num_v_fixed);
    vmap_surf2tet.resize(id1);
    for (size_t i = vmap_surf2tet.size(); i > 0; --i)
    {
        fin >> id1 >> id2;
        if (id2 >= vmap_surf2tet.size())
        {
            SIM_ERROR("Id2 is out of range in geometry file\n");
            exit(3);
        }
        vmap_surf2tet[id2] = id1;
    }
    if (fin.fail())
    {
        SIM_ERROR("Error occurred while reading file: %s\n", filename.c_str());
        exit(2);
    }
    fin.close();
}
#include "modal/ModalModel.h"
#include "protobuf/sploosh.pb.h"
#include "transfer/FMMTransferEval.h"

void load_moments(const std::string &filename,
                  std::vector<FMMTransferEval *> &transfer_, ModalModel *modal_)
{
    sploosh::ModalMoments mms;
    std::ifstream fin(filename.c_str(), std::ios::binary);
    if (!mms.ParseFromIstream(&fin))
    {
        SIM_ERROR("Cannot real protobuf file: %s\n", filename.c_str());
        SHOULD_NEVER_HAPPEN(-1);
    }

    const int nmms = (int)mms.moment_size();
    SIM_INFO("{} moments are detected\n", nmms);
    if (nmms < modal_->num_modes())
    {
        SIM_ERROR(
            "Number of moments (%d) is smaller than the number of modes (%d)\n",
            nmms, modal_->num_modes());
        SHOULD_NEVER_HAPPEN(-2);
    } // end if

    auto dir = cFileUtil::GetAbsPath(cFileUtil::GetDir(filename));

    // QFileInfo checkConfig(filename);
    // QString dir = checkConfig.absoluteDir().absolutePath(); //.<< endl;

    transfer_.resize(modal_->num_modes());
    for (int mi = 0; mi < modal_->num_modes(); ++mi)
    {
        transfer_[mi] = new FMMTransferEval(mms.moment(mi), dir);
    }
}

void cAcousticBody::InitAudioBuffer()
{
    // 1. load vertex map
    load_vertex_map(this->mVmapPath, mNumOfFixed, mVertexMapSurfaceToTet);

    // 2. create modal
    mModalModel = new ModalModel(this->mEigenPath, this->mAcousticProp.mRho,
                                 mAcousticProp.mA, mAcousticProp.mB);

    for (int nt = 0; nt < NUM_THREADS; ++nt)
        mForce_[nt].resize(mModalModel->num_modes());
    // 3. load moments
    load_moments(this->mMomentsPath, transfer_, this->mModalModel);

    // 4. create buffer
    // const long long len = SR * this->mClickInfo.mTS * 1 * 16 / 8;
    // this->buff
    for (int nt = 0; nt < NUM_THREADS; ++nt)
        soundBuffer_[nt].resize(SR * mClickInfo.mTS);

    whole_soundBuffer.resize(
        std::max(int(SR * 1.0 * 1 + SR * mClickInfo.mTS * 1), SR * 1));
}

void cAcousticBody::AudioSynthesis(bool enable_outsider_amp, _FLOAT outside_amp)
{
    for (auto &x : mClickInfo.mTriIds)
    {
        mTriangleArray[x]->mColor = ColorPurple;
    }
    std::thread ts[NUM_THREADS];
    printf("[warn] sel TRI id here!\n");
    std::vector<int> selTriIds = mClickInfo.mTriIds;
    std::vector<_FLOAT> Amps = mClickInfo.mAmp;

    SIM_ASSERT(selTriIds.size() == Amps.size());
    memset(whole_soundBuffer.data(), 0,
           sizeof(double) * whole_soundBuffer.size());

    for (int nt = 0; nt < NUM_THREADS; ++nt)
    {
        memset(soundBuffer_[nt].data(), 0,
               sizeof(double) * soundBuffer_[nt].size());
    }
    std::vector<_FLOAT> click_cam_pos = {};
    for (int i = 0; i < selTriIds.size(); ++i)
    {
        int index_t = i % NUM_THREADS;
        int selTriId = selTriIds[i]; // click的三角形id
        double amp = Amps[i] / 1e3;  // click的力大小, 缩小16倍
        // double collision_time = times.at(i).toDouble(); // 碰撞时间(起始时间)
        double collision_time = 0;

        // if (collision_time > duration) // 如果碰撞时间 > 3s, 忽略(为什么?)
        //     break;

        const std::vector<Point3d> &vtx = mesh_->vertices(); // 表面顶点位置
        const std::vector<Tuple3ui> &tgl =
            mesh_->surface_indices(); // 表面三角形对应的顶点id
        Vector3d nml =
            Triangle<double>::normal(vtx[tgl[selTriId].x], vtx[tgl[selTriId].y],
                                     vtx[tgl[selTriId].z]); // 三角形法向量
        // Vector3d
        // nml(normal1s.at(i).toDouble(),normal2s.at(i).toDouble(),normal3s.at(i).toDouble());
        nml.normalize();

        // get click point

        Point3d click_pos =
            vtx[tgl[selTriId].x] + vtx[tgl[selTriId].y] + vtx[tgl[selTriId].z];
        click_pos /= 3;

        Point3d CamPos; // 相机位置

        CamPos.x = mCamPos[0];
        CamPos.y = mCamPos[1];
        CamPos.z = mCamPos[2];

        _FLOAT dist = (click_pos - CamPos).norm();

        click_cam_pos.push_back(dist);
        printf("click pos %.3f %.3f %.3f, cam pos %.3f %.3f %.3f, dist %.3f\n",
               click_pos.x, click_pos.y, click_pos.z, CamPos.x, CamPos.y,
               CamPos.z, dist

        );
        printf("[debug] col time %.1f, tri id %d, nml %.1f %.1f %.1f, cam pos "
               "%.1f %.1f %.1f amp %.1f, index %d\n",
               collision_time, selTriId, nml.x, nml.y, nml.z, CamPos.x,
               CamPos.y, CamPos.z, amp, index_t);
        ts[index_t] = std::thread(&cAcousticBody::run_thread, this,
                                  collision_time, selTriId, nml, CamPos, amp,
                                  index_t); // 启动新进程, 传递当前this指针进去.
                                            // 该函数会修改 whole_soundBuffer
        running_threads += 1;

        // if( i%NUM_THREADS == NUM_THREADS-1 || i==selected_Tris.size()-1){
        if ((i == selTriIds.size() - 1) || i % NUM_THREADS == NUM_THREADS - 1)
        {
            for (int nt = 0; nt < running_threads; ++nt)
            {
                ts[nt].join(); // 等待线程停止
            }
            running_threads = 0;
        }
        //        audio_->single_channel_synthesis(mesh_.triangle_ids(selTriId),
        //        nml, CamPos, amp, index_t);

        // cout<<"======\n";

        //        for (int j=0; j < audio_->soundBuffer_[index_t].size(); ++j){
        //            whole_soundBuffer.at(SR*collision_time*audio_->format_.channelCount()
        //            + j) += audio_->soundBuffer_[index_t].at(j);
    }

    float max_sound = *(std::max_element(std::begin(whole_soundBuffer),
                                         std::end(whole_soundBuffer)));

    _FLOAT mean_cam_dist = 0;
    {
        for (auto &x : click_cam_pos)
        {
            mean_cam_dist += x;
        }
        mean_cam_dist /= click_cam_pos.size();
    }
    if (mEnableAudioScale == true)
    {
        float scale = 0.9 / max_sound;
        if (enable_outsider_amp == false)
        {
            _FLOAT dist_scale =
                cMathUtil::Clamp(-1.5 * mean_cam_dist + 26.3, 0.2f, 1.0f);
            // _FLOAT dist_scale = 1;
            scale *= dist_scale;
            printf("mean cam dist %.3f, apply scale %.1f, dist scale %.3f\n",
                   mean_cam_dist, scale, dist_scale);
        }
        else
        {
            scale *= outside_amp;
            printf("outside amp %.3f\n", outside_amp);
        }
        for (auto &i : whole_soundBuffer)
            i *= scale;
        max_sound = 0.9;
    }
    else
    {
        // normalize if too loud
        if (max_sound > 500)
        {
            max_sound = std::min(500.0f, max_sound);
            printf("max sound %.1f\n", max_sound);
            if (max_sound > 1.0)
            {
                for (auto &i : whole_soundBuffer)
                    i /= max_sound;
            }
        }
    }
    // convert it to audio wave
    mSynthesisAudio = std::make_shared<tDiscretedWave>(1.0 / SR);
    mSynthesisAudio->SetData(whole_soundBuffer);
    // wave->DumpToWAV("tmp.wav");

    auto output = cAudioOutput::getInstance();
    output->SetWave(mSynthesisAudio);

    printf("set wav succ\n");
}

void cAcousticBody::InitAudioGeo()
{
    mesh_ = new TriangleMesh<double>();
    if (0 != MeshObjReader::read(this->mSurfaceObjPath.c_str(), *mesh_))
    {
        SIM_ERROR("cannot load mesh {}", mSurfaceObjPath);
        exit(1);
    }
}

void quickSort(float arr[], int index[], int left, int right)
{
    int i = left, j = right;
    float tmp;
    int tmp_i;
    float pivot = arr[(left + right) / 2];
    /* partition */
    while (i <= j)
    {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        if (i <= j)
        {
            tmp = arr[i];
            tmp_i = index[i];
            arr[i] = arr[j];
            index[i] = index[j];
            arr[j] = tmp;
            index[j] = tmp_i;
            i++;
            j--;
        }
    };
    /* recursion */
    if (left < j)
        quickSort(arr, index, left, j);
    if (i < right)
        quickSort(arr, index, i, right);
}

void cAcousticBody::single_channel_synthesis(const Tuple3ui &tri,
                                             const Vector3d &dir,
                                             const Point3d &cam,
                                             float amplitude, int index_t)
{
    // force in modal space: U^T f_cartesian
    // 清理 mForce_[index_t]. 需要指定f_caresian中非0项的id,
    // 以及每个mode的特征向量的id对应项
    memset(mForce_[index_t].data(), 0,
           sizeof(double) * mForce_[index_t].size());
    // select a surface, then apply force to three vertices
    // force is double type
    // vidS2T is vertex map for surface VID to tet VID
    // tri is a surface ID
    // tri.x is a vertex ID

    mModalModel->accum_modal_impulse(
        mVertexMapSurfaceToTet[tri.x] - mNumOfFixed, &dir,
        mForce_[index_t].data()); // 三角形顶点0
    mModalModel->accum_modal_impulse(
        mVertexMapSurfaceToTet[tri.y] - mNumOfFixed, &dir,
        mForce_[index_t].data()); // 三角形顶点1
    mModalModel->accum_modal_impulse(
        mVertexMapSurfaceToTet[tri.z] - mNumOfFixed, &dir,
        mForce_[index_t].data()); // 三角形顶点2

    // 此时, mForce_中存储着当前这个方向的力 dir / rho 时的模态力(广义力).
    // mForce_ \in R^60
    const std::vector<double> &omegaD = mModalModel->damped_omega();
    const std::vector<double> &c = mModalModel->damping_vector();

    // multiply with the impulse response of each modes
    memset(soundBuffer_[index_t].data(), 0,
           sizeof(double) * soundBuffer_[index_t].size());
    const int totTicks = SR * mClickInfo.mTS;

    float sum_moments = 0;
    float sum_energy = 0; // 总能量
    float modes[mModalModel->num_modes()];
    int index[mModalModel->num_modes()];
    for (int i = 0; i < mModalModel->num_modes(); ++i)
    {
        sum_moments += abs(mForce_[index_t][i]);
        sum_energy += abs(mForce_[index_t][i]) *
                      abs(mForce_[index_t][i]); // 力的平方就是总能量
        modes[i] = abs(mForce_[index_t][i]); // modes[i]存储了每个模态的激发程度
        index[i] = i;
    }
    quickSort(
        modes, index, 0,
        mModalModel->num_modes() -
            1); // modes 按照激发程度, 从小到大排列. 并且记录排序后的index[]
    int num_modes = 0;

    // int seq=1;
    // string name="modes1_norm.txt";
    // while(exists_test(name)){
    //   seq++;
    //   string name_seq = to_string(seq);
    //   name = "modes_norm"+name_seq+".txt";
    // }
    // ofstream modesfile;
    // modesfile.open(name);

    float current_sum = 0;
    float current_energy = 0;

    // 对于每一个mode, 从小能量的Mode开始计算
    for (int j = 0; j < mModalModel->num_modes(); ++j)
    {
        // 获得其索引
        int i = index[mModalModel->num_modes() - 1 - j];
        if (current_energy <=
            sum_energy * 0.9) // 如果当前能量还不够90%, 则继续计算
        {
            if (omegaD[i] < 120)
            {
                printf("[debug] mode %d wd %.1f < 120, ignore\n", omegaD[i]);
                continue;
            }
            // 加入当前的能量
            current_energy +=
                abs(mForce_[index_t][i]) * abs(mForce_[index_t][i]);
            FMMTransferEval::TComplex trans = transfer_[i]->eval(
                cam); // 对于这个mode, 评估其cam位置的声强传递函数

            num_modes += 1;
            // modesfile << abs(mForce_[i]) << "\n";
            /*
                SS = (U^T * f / rho)[i] * trans / wd * 0.4
            */
            const double SS =
                mForce_[index_t][i] * abs(trans) / omegaD[i] * 0.4;
            // cout<<"mode "<<i<<" mForce: "<<mForce_[i]<<"\n";
            // clock_t sub_start = clock();
            printf("[debug] mode %d w %.1f trans %.3f c %.3f\n", j, omegaD[i],
                   abs(trans), c[i] * mCustomDampVectorScale);
            for (int ti = 0; ti < totTicks; ++ti)
            {
                const double ts =
                    static_cast<double>(ti) / static_cast<double>(SR); // time
                const double amp =
                    exp(-c[i] * 0.5 * ts *
                        mCustomDampVectorScale); // exp(-xi * omega * t)
                /*
                    sound += e^{c_i * 0.5 * t} * SS * sin(wd * t) * amplitude
                                exp decay unit_force_sound_pressure_this_mode *
                   force * sin
                */
                if (amp < 1E-3)
                    break;
                double result = amp * SS * sin(omegaD[i] * ts) *
                                amplitude; // sin(omega_d * t);
                soundBuffer_[index_t][ti] += result;
                if (std::isnan(result) == true)
                {
                    result = 0;
                    printf(
                        "amp %.1f ss %.1f omegaD %.1f ts %.1f amplitude %.1f\n",
                        amp, SS, omegaD[i], ts, amplitude);
                    assert(false);
                }
            }
            // clock_t sub_end = clock();
            // long sub_elapsed_secs = long(sub_end - sub_start);
            // cout<<"sub time a click: "<<sub_elapsed_secs<<endl;
            //
        }
        // else {break;}
    } // end for
}

void cAcousticBody::run_thread(double collision_time, int selTriId,
                               Vector3d nml, Point3d CamPos, float amp,
                               int index_t)
{
    this->single_channel_synthesis(mesh_->triangle_ids(selTriId), nml, CamPos,
                                   amp, index_t);

    // std::lock_guard<std::mutex> lock(write_buffer_lock);
    // 对于合成出来的声音buffer的size
    for (int j = 0; j < soundBuffer_[index_t].size(); ++j)
    {
        // 原始位置: j
        // 目标位置: int(SR * 碰撞时间 * 通道数1 + j).
        // 也就是从碰撞发生时开始的一段声音的写入
        whole_soundBuffer.at(SR * collision_time * 1 + j) +=
            this->soundBuffer_[index_t].at(j);
    }
}

#include "imgui.h"
void cAcousticBody::UpdateImGui()
{
    /*
        show current info
        1. cam pos; rho, a, b
        2. click v set
        3. audio amp, ts
    */

    ImGui::DragFloat("custom damp", &mCustomDampVectorScale, 0, 0.1, 10,
                     "%.1f");
    if (ImGui::CollapsingHeader("acoustic options",
                                ImGuiTreeNodeFlags_DefaultOpen))
    {

        ImGui::Text("cam pos %.3f %.3f %.3f", mCamPos[0], mCamPos[1],
                    mCamPos[2]);

        {
            std::string format_string = "click vertex num %d, id";
            for (int i = 0; i < mClickInfo.mTriIds.size(); i++)
            {
                format_string += " " + std::to_string(mClickInfo.mTriIds[i]);
            }
            ImGui::Text(format_string.c_str(), mClickInfo.mTriIds.size());
        }

        ImGui::Text("audio amp %.1f ts %.1f", mClickInfo.mAudioAmp,
                    mClickInfo.mTS);
        ImGui::Checkbox("enable range click", &mEnableRangeClick);
        ImGui::Checkbox("enable audio scale", &mEnableAudioScale);
        if (mEnableRangeClick)
        {
            // R = 1mm - 1cm
            ImGui::DragFloat("click R", &mRangeClickRadius, 1e-4, 1e-3, 3e-3,
                             "%.4f");
        }
    }
}

#include <queue>
std::vector<int> find_min_k(const std::vector<double> &test, int k = 5)
{
    std::vector<int> indices(test.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::partial_sort(indices.begin(), indices.begin() + k, indices.end(),
                      [&](int A, int B) { return test[A] < test[B]; });
    std::vector<int> vals(0);
    for (int j = 0; j < k; j++)
    {
        vals.push_back(indices[j]);
    }
    return vals;
}

#include "sim/Perturb.h"
void cAcousticBody::ApplyUserPerturbForceOnce(tPerturb *pert)
{
    bool need_to_reproduce = false;
    std::vector<int> click_ids(0);
    // 1. calculate current click vertex
    if (mEnableRangeClick == true)
    {
        int center_tid = pert->mAffectedTriId;
        std::vector<_FLOAT> dist(GetNumOfTriangles(), 0);
        for (int i = 0; i < GetNumOfTriangles(); i++)
            dist[i] = (mTrianglesCOM[center_tid] - mTrianglesCOM[i]).norm();
        // std::cout << "click pos = " << mTrianglesCOM[center_tid].transpose()
        //           << std::endl;
        // trial to
        int init_trial = 5;
        while (true)
        {
            std::vector<int> indices = find_min_k(dist, init_trial);
            // for (auto i : indices)
            // {
            //     printf("tri %d dist %.3f pos %.3f %.3f %.3f\n", i, dist[i],
            //            mTrianglesCOM[i][0], mTrianglesCOM[i][1],
            //            mTrianglesCOM[i][2]);
            // }
            // std::cout << "trial " << init_trial << std::endl;
            if (dist[indices[indices.size() - 1]] > mRangeClickRadius)
            {

                for (int j = 0; j < init_trial; j++)
                {
                    if (dist[indices[j]] < mRangeClickRadius)
                    {
                        click_ids.push_back(indices[j]);
                        // std::cout << "click " << click_ids[j]
                        //           << " dist = " << dist[indices[j]]
                        //           << std::endl;
                    }
                    else
                    {
                        // std::cout << "num of clicks = " << click_ids.size()
                        //           << std::endl;
                        break;
                    }
                }
                break;
            }
            else
            {

                init_trial *= 2;
            }
        }
    }
    else
    {
        click_ids = {pert->mAffectedTriId};
    }

    if (mClickInfo.mTriIds.size() != click_ids.size())
    {
        need_to_reproduce = true;
    }
    else
    {
        for (int i = 0; i < click_ids.size(); i++)
        {
            if (mClickInfo.mTriIds[i] != click_ids[i])
            {
                need_to_reproduce = true;
                break;
            }
        }
    }
    //
    if (need_to_reproduce)
    {
        std::cout << "[debug] recalc audio for " << click_ids.size()
                  << " pts: ";
        for (auto &x : click_ids)
        {
            std::cout << x << " ";
        }
        std::cout << std::endl;

        // old triangles set to blue
        for (auto &x : mClickInfo.mTriIds)
        {
            mTriangleArray[x]->mColor = ColorBlue;
        }

        // new triangles set to gray

        mClickInfo.mTriIds = click_ids;
        mClickInfo.mAmp.clear();
        for (auto &i : click_ids)
        {
            mClickInfo.mAmp.push_back(1);
        }

        AudioSynthesis();
        // for (int i = 0; i < whole_soundBuffer.size(); i++)
        // {

        //     whole_soundBuffer[i] =
        //         std::sin(std::sin(2 * M_PI * 1000 * i * 1.0 / SR));
        // }
        // auto wave = std::make_shared<tDiscretedWave>(1.0 / SR);
        // wave->SetData(whole_soundBuffer);
        // // wave->DumpToWAV("tmp.wav");

        // auto output = cAudioOutput::getInstance();
        // output->SetWave(wave);
    }
}

void cAcousticBody::Shift(const tVector3 &pos)
{
    for (auto &v : mVertexArray)
    {
        v->mPos.segment(0, 3) += pos;
    }
}

std::string cAcousticBody::GetIniPath() const { return this->mIniPath; }
tVectorXf cAcousticBody::GetVertexPosVec()
{
    tVectorXf vec_pos(3 * GetNumOfVertices());
    for (int vid = 0; vid < GetNumOfVertices(); vid++)
    {
        vec_pos.segment(3 * vid, 3) =
            mVertexArray[vid]->mPos.segment(0, 3).cast<float>();
    }
    return vec_pos;
}
tVectorXi cAcousticBody::GetTriIdVec()
{
    tVectorXi vec(3 * GetNumOfTriangles());
    for (int i = 0; i < GetNumOfTriangles(); i++)
    {
        vec[3 * i + 0] = mTriangleArray[i]->mId0;
        vec[3 * i + 1] = mTriangleArray[i]->mId1;
        vec[3 * i + 2] = mTriangleArray[i]->mId2;
    }
    return vec;
}
void cAcousticBody::UpdateCamPos(const tVector3f &pos)
{
    mCamPos = pos.cast<_FLOAT>();
}
void cAcousticBody::ClickTriangle(int tid, _FLOAT outside_scale,
                                  _FLOAT tmp_overdamp_scale)
{
    float cur_tmp = mCustomDampVectorScale;
    mCustomDampVectorScale = tmp_overdamp_scale;
    mClickInfo.mTS;
    mClickInfo.mAudioAmp = 1;
    mClickInfo.mAmp = {1};
    mClickInfo.mNormal = {tVector3(0, 1, 0)};
    mClickInfo.mTriIds = {tid};
    std::cout << "custom damp scale = " << mCustomDampVectorScale << std::endl;
    AudioSynthesis(true, outside_scale);
    mCustomDampVectorScale = cur_tmp;
}

tVector3f cAcousticBody::GetCamPos() const
{
    return this->mCamPos.cast<float>();
}