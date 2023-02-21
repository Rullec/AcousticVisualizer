#include "MathUtil.h"
#include "LogUtil.h"
#include "utils/DefUtil.h"
#include <iostream>
#include <time.h>
// const enum eRotationOrder gRotationOrder = eRotationOrder::XYZ;
// const tVector gGravity = tVector4(0, -9.8, 0, 0);
// const tVector gGravity = tVector4(0, 0, 0, 0);
cRand cMathUtil::gRand = cRand();

bool cMathUtil::IsPoint(const tVector4 &vec)
{
    return std::fabs(vec[3] - 1.0) < 1e-10;
}
tVector4 cMathUtil::VecToPoint(const tVector4 &vec)
{
    tVector4 new_vec = vec;
    new_vec[3] = 1;
    return new_vec;
}
int cMathUtil::Clamp(int val, int min, int max)
{
    return SIM_MAX(min, SIM_MIN(val, max));
}

// void cMathUtil::Clamp(const tVectorXd &min, const tVectorXd &max,
//                       tVectorXd &out_vec)
// {
//     out_vec = out_vec.cwiseMin(max).cwiseMax(min);
// }

_FLOAT cMathUtil::Clamp(_FLOAT val, _FLOAT min, _FLOAT max)
{
    return SIM_MAX(min, SIM_MIN(val, max));
}

_FLOAT cMathUtil::Saturate(_FLOAT val) { return Clamp(val, 0.0, 1.0); }

_FLOAT cMathUtil::Lerp(_FLOAT t, _FLOAT val0, _FLOAT val1)
{
    return (1 - t) * val0 + t * val1;
}

_FLOAT cMathUtil::NormalizeAngle(_FLOAT theta)
{
    // normalizes theta to be between [-pi, pi]
    _FLOAT norm_theta = fmod(theta, 2 * M_PI);
    if (norm_theta > M_PI)
    {
        norm_theta = -2 * M_PI + norm_theta;
    }
    else if (norm_theta < -M_PI)
    {
        norm_theta = 2 * M_PI + norm_theta;
    }
    return norm_theta;
}

_FLOAT cMathUtil::RandFloat() { return RandFloat(0, 1); }

_FLOAT cMathUtil::RandFloat(_FLOAT min, _FLOAT max)
{
    return gRand.RandFloat(min, max);
}

_FLOAT cMathUtil::RandFloatNorm(_FLOAT mean, _FLOAT stdev)
{
    return gRand.RandFloatNorm(mean, stdev);
}

_FLOAT cMathUtil::RandFloatExp(_FLOAT lambda)
{
    return gRand.RandFloatExp(lambda);
}

_FLOAT cMathUtil::RandFloatSeed(_FLOAT seed)
{
    unsigned int int_seed = *reinterpret_cast<unsigned int *>(&seed);
    std::default_random_engine rand_gen(int_seed);
    std::uniform_real_distribution<_FLOAT> dist;
    return dist(rand_gen);
}

int cMathUtil::RandInt() { return gRand.RandInt(); }

int cMathUtil::RandInt(int min, int max) { return gRand.RandInt(min, max); }

int cMathUtil::RandUint() { return gRand.RandUint(); }

int cMathUtil::RandUint(unsigned int min, unsigned int max)
{
    return gRand.RandUint(min, max);
}

int cMathUtil::RandIntExclude(int min, int max, int exc)
{
    return gRand.RandIntExclude(min, max, exc);
}

void cMathUtil::SeedRand(unsigned long int seed)
{
    gRand.Seed(seed);
    srand(gRand.RandInt());
}

int cMathUtil::RandSign() { return gRand.RandSign(); }

_FLOAT cMathUtil::SmoothStep(_FLOAT t)
{
    _FLOAT val = t * t * t * (t * (t * 6 - 15) + 10);
    return val;
}

bool cMathUtil::FlipCoin(_FLOAT p) { return gRand.FlipCoin(p); }


_FLOAT cMathUtil::Sign(_FLOAT val) { return SignAux<_FLOAT>(val); }

int cMathUtil::Sign(int val) { return SignAux<int>(val); }

_FLOAT cMathUtil::AddAverage(_FLOAT avg0, int count0, _FLOAT avg1, int count1)
{
    _FLOAT total = count0 + count1;
    return (count0 / total) * avg0 + (count1 / total) * avg1;
}

tVector4 cMathUtil::AddAverage(const tVector4 &avg0, int count0,
                              const tVector4 &avg1, int count1)
{
    _FLOAT total = count0 + count1;
    return (count0 / total) * avg0 + (count1 / total) * avg1;
}

// void cMathUtil::AddAverage(const tVectorXd &avg0, int count0,
//                            const tVectorXd &avg1, int count1,
//                            tVectorXd &out_result)
// {
//     _FLOAT total = count0 + count1;
//     out_result = (count0 / total) * avg0 + (count1 / total) * avg1;
// }

// void cMathUtil::CalcSoftmax(const tVectorXd &vals, _FLOAT temp,
//                             tVectorXd &out_prob)
// {
//     assert(out_prob.size() == vals.size());
//     int num_vals = static_cast<int>(vals.size());
//     _FLOAT sum = 0;
//     _FLOAT max_val = vals.maxCoeff();
//     for (int i = 0; i < num_vals; ++i)
//     {
//         _FLOAT val = vals[i];
//         val = std::exp((val - max_val) / temp);
//         out_prob[i] = val;
//         sum += val;
//     }

//     out_prob /= sum;
// }

// _FLOAT cMathUtil::EvalGaussian(const tVectorXd &mean,
//                                const tVectorXd &covar,
//                                const tVectorXd &sample)
// {
//     assert(mean.size() == covar.size());
//     assert(sample.size() == covar.size());

//     tVectorXd diff = sample - mean;
//     _FLOAT exp_val = diff.dot(diff.cwiseQuotient(covar));
//     _FLOAT likelihood = std::exp(-0.5 * exp_val);

//     _FLOAT partition = CalcGaussianPartition(covar);
//     likelihood /= partition;
//     return likelihood;
// }

// _FLOAT cMathUtil::EvalGaussian(_FLOAT mean, _FLOAT covar, _FLOAT sample)
// {
//     _FLOAT diff = sample - mean;
//     _FLOAT exp_val = diff * diff / covar;
//     _FLOAT norm = 1 / std::sqrt(2 * M_PI * covar);
//     _FLOAT likelihood = norm * std::exp(-0.5 * exp_val);
//     return likelihood;
// }

// _FLOAT cMathUtil::CalcGaussianPartition(const tVectorXd &covar)
// {
//     int data_size = static_cast<int>(covar.size());
//     _FLOAT det = covar.prod();
//     _FLOAT partition = std::sqrt(std::pow(2 * M_PI, data_size) * det);
//     return partition;
// }

// _FLOAT cMathUtil::EvalGaussianLogp(const tVectorXd &mean,
//                                    const tVectorXd &covar,
//                                    const tVectorXd &sample)
// {
//     int data_size = static_cast<int>(covar.size());

//     tVectorXd diff = sample - mean;
//     _FLOAT logp = -0.5 * diff.dot(diff.cwiseQuotient(covar));
//     _FLOAT det = covar.prod();
//     logp += -0.5 * (data_size * std::log(2 * M_PI) + std::log(det));

//     return logp;
// }

// _FLOAT cMathUtil::EvalGaussianLogp(_FLOAT mean, _FLOAT covar, _FLOAT sample)
// {
//     _FLOAT diff = sample - mean;
//     _FLOAT logp = -0.5 * diff * diff / covar;
//     logp += -0.5 * (std::log(2 * M_PI) + std::log(covar));
//     return logp;
// }

// _FLOAT cMathUtil::Sigmoid(_FLOAT x) { return Sigmoid(x, 1, 0); }

// _FLOAT cMathUtil::Sigmoid(_FLOAT x, _FLOAT gamma, _FLOAT bias)
// {
//     _FLOAT exp = -gamma * (x + bias);
//     _FLOAT val = 1 / (1 + std::exp(exp));
//     return val;
// }

// int cMathUtil::SampleDiscreteProb(const tVectorX &probs)
// {
//     assert(std::abs(probs.sum() - 1) < 0.00001);
//     _FLOAT rand = RandFloat();

//     int rand_idx = gInvalidIdx;
//     int num_probs = static_cast<int>(probs.size());
//     for (int i = 0; i < num_probs; ++i)
//     {
//         _FLOAT curr_prob = probs[i];
//         rand -= curr_prob;

//         if (rand <= 0)
//         {
//             rand_idx = i;
//             break;
//         }
//     }
//     return rand_idx;
// }

/**
 * \briewf          categorical random distribution
 */
int cMathUtil::RandIntCategorical(const std::vector<_FLOAT> &prop)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d(prop.begin(), prop.end());
    int num = d(gen);
    return num;
}

tVector4 cMathUtil::CalcBarycentric(const tVector4 &p, const tVector4 &a,
                                   const tVector4 &b, const tVector4 &c)
{
    tVector4 v0 = b - a;
    tVector4 v1 = c - a;
    tVector4 v2 = p - a;

    _FLOAT d00 = v0.dot(v0);
    _FLOAT d01 = v0.dot(v1);
    _FLOAT d11 = v1.dot(v1);
    _FLOAT d20 = v2.dot(v0);
    _FLOAT d21 = v2.dot(v1);
    _FLOAT denom = d00 * d11 - d01 * d01;
    _FLOAT v = (d11 * d20 - d01 * d21) / denom;
    _FLOAT w = (d00 * d21 - d01 * d20) / denom;
    _FLOAT u = 1.0f - v - w;
    return tVector4(u, v, w, 0);
}

bool cMathUtil::ContainsAABB(const tVector4 &pt, const tVector4 &aabb_min,
                             const tVector4 &aabb_max)
{
    bool contains = pt[0] >= aabb_min[0] && pt[1] >= aabb_min[1] &&
                    pt[2] >= aabb_min[2] && pt[0] <= aabb_max[0] &&
                    pt[1] <= aabb_max[1] && pt[2] <= aabb_max[2];
    return contains;
}

bool cMathUtil::ContainsAABB(const tVector4 &aabb_min0, const tVector4 &aabb_max0,
                             const tVector4 &aabb_min1, const tVector4 &aabb_max1)
{
    return ContainsAABB(aabb_min0, aabb_min1, aabb_max1) &&
           ContainsAABB(aabb_max0, aabb_min1, aabb_max1);
}

bool cMathUtil::ContainsAABBXZ(const tVector4 &pt, const tVector4 &aabb_min,
                               const tVector4 &aabb_max)
{
    bool contains = pt[0] >= aabb_min[0] && pt[2] >= aabb_min[2] &&
                    pt[0] <= aabb_max[0] && pt[2] <= aabb_max[2];
    return contains;
}

bool cMathUtil::ContainsAABBXZ(const tVector4 &aabb_min0,
                               const tVector4 &aabb_max0,
                               const tVector4 &aabb_min1,
                               const tVector4 &aabb_max1)
{
    return ContainsAABBXZ(aabb_min0, aabb_min1, aabb_max1) &&
           ContainsAABBXZ(aabb_max0, aabb_min1, aabb_max1);
}

void cMathUtil::CalcAABBIntersection(const tVector4 &aabb_min0,
                                     const tVector4 &aabb_max0,
                                     const tVector4 &aabb_min1,
                                     const tVector4 &aabb_max1, tVector4 &out_min,
                                     tVector4 &out_max)
{
    out_min = aabb_min0.cwiseMax(aabb_min1);
    out_max = aabb_max0.cwiseMin(aabb_max1);
    if (out_min[0] > out_max[0])
    {
        out_min[0] = 0;
        out_max[0] = 0;
    }
    if (out_min[1] > out_max[1])
    {
        out_min[1] = 0;
        out_max[1] = 0;
    }
    if (out_min[2] > out_max[2])
    {
        out_min[2] = 0;
        out_max[2] = 0;
    }
}

void cMathUtil::CalcAABBUnion(const tVector4 &aabb_min0,
                              const tVector4 &aabb_max0,
                              const tVector4 &aabb_min1,
                              const tVector4 &aabb_max1, tVector4 &out_min,
                              tVector4 &out_max)
{
    out_min = aabb_min0.cwiseMin(aabb_min1);
    out_max = aabb_max0.cwiseMax(aabb_max1);
}

bool cMathUtil::IntersectAABB(const tVector4 &aabb_min0,
                              const tVector4 &aabb_max0,
                              const tVector4 &aabb_min1,
                              const tVector4 &aabb_max1)
{
    tVector4 center0 = 0.5 * (aabb_max0 + aabb_min0);
    tVector4 center1 = 0.5 * (aabb_max1 + aabb_min1);
    tVector4 size0 = aabb_max0 - aabb_min0;
    tVector4 size1 = aabb_max1 - aabb_min1;
    tVector4 test_len = 0.5 * (size0 + size1);
    tVector4 delta = center1 - center0;
    bool overlap = (std::abs(delta[0]) <= test_len[0]) &&
                   (std::abs(delta[1]) <= test_len[1]) &&
                   (std::abs(delta[2]) <= test_len[2]);
    return overlap;
}

bool cMathUtil::IntersectAABBXZ(const tVector4 &aabb_min0,
                                const tVector4 &aabb_max0,
                                const tVector4 &aabb_min1,
                                const tVector4 &aabb_max1)
{
    tVector4 center0 = 0.5 * (aabb_max0 + aabb_min0);
    tVector4 center1 = 0.5 * (aabb_max1 + aabb_min1);
    tVector4 size0 = aabb_max0 - aabb_min0;
    tVector4 size1 = aabb_max1 - aabb_min1;
    tVector4 test_len = 0.5 * (size0 + size1);
    tVector4 delta = center1 - center0;
    bool overlap = (std::abs(delta[0]) <= test_len[0]) &&
                   (std::abs(delta[2]) <= test_len[2]);
    return overlap;
}

bool cMathUtil::CheckNextInterval(_FLOAT delta, _FLOAT curr_val,
                                  _FLOAT int_size)
{
    _FLOAT pad = 0.001 * delta;
    int curr_count = static_cast<int>(std::floor((curr_val + pad) / int_size));
    int prev_count =
        static_cast<int>(std::floor((curr_val + pad - delta) / int_size));
    bool new_action = (curr_count != prev_count);
    return new_action;
}

tVector4 cMathUtil::SampleRandPt(const tVector4 &bound_min,
                                const tVector4 &bound_max)
{
    tVector4 pt = tVector4(RandFloat(bound_min[0], bound_max[0]),
                         RandFloat(bound_min[1], bound_max[1]),
                         RandFloat(bound_min[2], bound_max[2]), 0);
    return pt;
}

tVector4 cMathUtil::SampleRandPtBias(const tVector4 &bound_min,
                                    const tVector4 &bound_max)
{
    return SampleRandPtBias(bound_min, bound_max,
                            0.5 * (bound_max + bound_min));
}

tVector4 cMathUtil::SampleRandPtBias(const tVector4 &bound_min,
                                    const tVector4 &bound_max,
                                    const tVector4 &focus)
{
    _FLOAT t = RandFloat(0, 1);
    tVector4 size = bound_max - bound_min;
    tVector4 new_min = focus + (t * 0.5) * size;
    tVector4 new_max = focus - (t * 0.5) * size;
    tVector4 offset = (bound_min - new_min).cwiseMax(0);
    offset += (bound_max - new_max).cwiseMin(0);
    new_min += offset;
    new_max += offset;

    return SampleRandPt(new_min, new_max);
}

// tQuaterniondd cMathUtil::RotMatToQuaternion(const tMatrix &mat)
//{
//	//
// http://www.iri.upc.edu/files/scidoc/2068-Accurate-Computation-of-Quaternions-from-Rotation-Matrices.pdf
//	_FLOAT eta = 0;
//	_FLOAT q1, q2, q3, q4;	// = [w, x, y, z]
//
//	// determine q1
//	{
//		_FLOAT detect_value = mat(0, 0) + mat(1, 1) + mat(2, 2);
//		if (detect_value > eta)
//		{
//			q1 = 0.5 * std::sqrt(1 + detect_value);
//		}
//		else
//		{
//			_FLOAT numerator = 0;
//			numerator += std::pow(mat(2, 1) - mat(1, 2), 2);
//			numerator += std::pow(mat(0, 2) - mat(2, 0), 2);
//			numerator += std::pow(mat(1, 0) - mat(0, 1), 2);
//			q1 = 0.5 *  std::sqrt(numerator / (3 - detect_value));
//		}
//	}
//
//	// determine q2
//	{
//		_FLOAT detect_value = mat(0, 0) - mat(1, 1) - mat(2, 2);
//		if (detect_value > eta)
//		{
//			q2 = 0.5 * std::sqrt(1 + detect_value);
//		}
//		else
//		{
//			_FLOAT numerator = 0;
//			numerator += std::pow(mat(2, 1) - mat(1, 2), 2);
//			numerator += std::pow(mat(0, 1) + mat(1, 0), 2);
//			numerator += std::pow(mat(2, 0) + mat(0, 2), 2);
//			q2 = 0.5 * std::sqrt(numerator / (3 - detect_value));
//		}
//	}
//
//	// determine q3
//	{
//		_FLOAT detect_value = -mat(0, 0) + mat(1, 1) - mat(2, 2);
//		if (detect_value > eta)
//		{
//			q3 = 0.5 * std::sqrt(1 + detect_value);
//		}
//		else
//		{
//			_FLOAT numerator = 0;
//			numerator += std::pow(mat(0, 2) - mat(2, 0), 2);
//			numerator += std::pow(mat(0, 1) + mat(1, 0), 2);
//			numerator += std::pow(mat(1, 2) + mat(2, 1), 2);
//			q3 = 0.5 * std::sqrt(numerator / (3 - detect_value));
//		}
//	}
//
//	// determine q4
//	{
//		_FLOAT detect_value = -mat(0, 0) - mat(1, 1) + mat(2, 2);
//		if (detect_value > eta)
//		{
//			q4 = 0.5 * std::sqrt(1 + detect_value);
//		}
//		else
//		{
//			_FLOAT numerator = 0;
//			numerator += std::pow(mat(1, 0) - mat(0, 1), 2);
//			numerator += std::pow(mat(2, 0) + mat(0, 2), 2);
//			numerator += std::pow(mat(2, 1) + mat(1, 2), 2);
//			q4 = 0.5 * std::sqrt(numerator / (3 - detect_value));
//		}
//	}
//
//	return tQuaterniondd(q1, q2, q3, q4);
//}
