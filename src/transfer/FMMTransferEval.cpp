#include "FMMTransferEval.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include "utils/term_msg.h"
#include "utils/macros.h"
#include "protobuf/sploosh.pb.h"
#include "io/MatrixIO.hpp"
#include "utils/LogUtil.h"

using namespace std;

FMMTransferEval::FMMTransferEval(const char *d) : moments_(NULL)
{
    namespace fs = boost::filesystem;

    fs::path dir(d);
    fs::path protofile = dir / "moments.pbuf";
    fs::path datafile = dir / "moments.dat";

    if (!fs::exists(protofile) || !fs::exists(datafile))
        exit(-1);

    sploosh::FMMoments moments;
    ifstream fin(protofile.string().c_str(), ios::binary);
    if (!moments.ParseFromIstream(&fin))
        exit(-1);

    nexpan_ = (int)moments.numexp();
    waveNum_ = moments.wavenum();
    center_.set(moments.center().x(), moments.center().y(), moments.center().z()); // the center of AABB
    moments_ = load_ma_matrixd(datafile.string().c_str());
}

/*
 * Load the moment data, assuming the filename is given in the FMMoments instance,
 * and the file is stored in the given directory.
 */
FMMTransferEval::FMMTransferEval(const sploosh::FMMoments &ms, const string &d) : moments_(NULL)
{
    namespace fs = boost::filesystem;
    fs::path dir(d);

    nexpan_ = (int)ms.numexp();
    waveNum_ = ms.wavenum();
    center_.set(ms.center().x(), ms.center().y(), ms.center().z());

    fs::path datafile = dir / ms.mfile();
    moments_ = load_ma_matrixd(datafile.string().c_str());
    if (!moments_)
    {
        SIM_ERROR("Failed to load the moment file: %s\n", datafile.string().c_str());
        exit(-4);
    }
    // SIM_INFO("load moment file: %s\n", datafile.string().c_str());
}
