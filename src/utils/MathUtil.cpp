#include "MathUtil.h"
#include "LogUtil.h"
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

FLOAT cMathUtil::Clamp(FLOAT val, FLOAT min, FLOAT max)
{
    return SIM_MAX(min, SIM_MIN(val, max));
}

FLOAT cMathUtil::Saturate(FLOAT val) { return Clamp(val, 0.0, 1.0); }

FLOAT cMathUtil::Lerp(FLOAT t, FLOAT val0, FLOAT val1)
{
    return (1 - t) * val0 + t * val1;
}

FLOAT cMathUtil::NormalizeAngle(FLOAT theta)
{
    // normalizes theta to be between [-pi, pi]
    FLOAT norm_theta = fmod(theta, 2 * M_PI);
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

FLOAT cMathUtil::RandFloat() { return RandFloat(0, 1); }

FLOAT cMathUtil::RandFloat(FLOAT min, FLOAT max)
{
    return gRand.RandFloat(min, max);
}

FLOAT cMathUtil::RandFloatNorm(FLOAT mean, FLOAT stdev)
{
    return gRand.RandFloatNorm(mean, stdev);
}

FLOAT cMathUtil::RandFloatExp(FLOAT lambda)
{
    return gRand.RandFloatExp(lambda);
}

FLOAT cMathUtil::RandFloatSeed(FLOAT seed)
{
    unsigned int int_seed = *reinterpret_cast<unsigned int *>(&seed);
    std::default_random_engine rand_gen(int_seed);
    std::uniform_real_distribution<FLOAT> dist;
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

FLOAT cMathUtil::SmoothStep(FLOAT t)
{
    FLOAT val = t * t * t * (t * (t * 6 - 15) + 10);
    return val;
}

bool cMathUtil::FlipCoin(FLOAT p) { return gRand.FlipCoin(p); }


FLOAT cMathUtil::Sign(FLOAT val) { return SignAux<FLOAT>(val); }

int cMathUtil::Sign(int val) { return SignAux<int>(val); }

FLOAT cMathUtil::AddAverage(FLOAT avg0, int count0, FLOAT avg1, int count1)
{
    FLOAT total = count0 + count1;
    return (count0 / total) * avg0 + (count1 / total) * avg1;
}

tVector4 cMathUtil::AddAverage(const tVector4 &avg0, int count0,
                              const tVector4 &avg1, int count1)
{
    FLOAT total = count0 + count1;
    return (count0 / total) * avg0 + (count1 / total) * avg1;
}

// void cMathUtil::AddAverage(const tVectorXd &avg0, int count0,
//                            const tVectorXd &avg1, int count1,
//                            tVectorXd &out_result)
// {
//     FLOAT total = count0 + count1;
//     out_result = (count0 / total) * avg0 + (count1 / total) * avg1;
// }

// void cMathUtil::CalcSoftmax(const tVectorXd &vals, FLOAT temp,
//                             tVectorXd &out_prob)
// {
//     assert(out_prob.size() == vals.size());
//     int num_vals = static_cast<int>(vals.size());
//     FLOAT sum = 0;
//     FLOAT max_val = vals.maxCoeff();
//     for (int i = 0; i < num_vals; ++i)
//     {
//         FLOAT val = vals[i];
//         val = std::exp((val - max_val) / temp);
//         out_prob[i] = val;
//         sum += val;
//     }

//     out_prob /= sum;
// }

// FLOAT cMathUtil::EvalGaussian(const tVectorXd &mean,
//                                const tVectorXd &covar,
//                                const tVectorXd &sample)
// {
//     assert(mean.size() == covar.size());
//     assert(sample.size() == covar.size());

//     tVectorXd diff = sample - mean;
//     FLOAT exp_val = diff.dot(diff.cwiseQuotient(covar));
//     FLOAT likelihood = std::exp(-0.5 * exp_val);

//     FLOAT partition = CalcGaussianPartition(covar);
//     likelihood /= partition;
//     return likelihood;
// }

// FLOAT cMathUtil::EvalGaussian(FLOAT mean, FLOAT covar, FLOAT sample)
// {
//     FLOAT diff = sample - mean;
//     FLOAT exp_val = diff * diff / covar;
//     FLOAT norm = 1 / std::sqrt(2 * M_PI * covar);
//     FLOAT likelihood = norm * std::exp(-0.5 * exp_val);
//     return likelihood;
// }

// FLOAT cMathUtil::CalcGaussianPartition(const tVectorXd &covar)
// {
//     int data_size = static_cast<int>(covar.size());
//     FLOAT det = covar.prod();
//     FLOAT partition = std::sqrt(std::pow(2 * M_PI, data_size) * det);
//     return partition;
// }

// FLOAT cMathUtil::EvalGaussianLogp(const tVectorXd &mean,
//                                    const tVectorXd &covar,
//                                    const tVectorXd &sample)
// {
//     int data_size = static_cast<int>(covar.size());

//     tVectorXd diff = sample - mean;
//     FLOAT logp = -0.5 * diff.dot(diff.cwiseQuotient(covar));
//     FLOAT det = covar.prod();
//     logp += -0.5 * (data_size * std::log(2 * M_PI) + std::log(det));

//     return logp;
// }

// FLOAT cMathUtil::EvalGaussianLogp(FLOAT mean, FLOAT covar, FLOAT sample)
// {
//     FLOAT diff = sample - mean;
//     FLOAT logp = -0.5 * diff * diff / covar;
//     logp += -0.5 * (std::log(2 * M_PI) + std::log(covar));
//     return logp;
// }

// FLOAT cMathUtil::Sigmoid(FLOAT x) { return Sigmoid(x, 1, 0); }

// FLOAT cMathUtil::Sigmoid(FLOAT x, FLOAT gamma, FLOAT bias)
// {
//     FLOAT exp = -gamma * (x + bias);
//     FLOAT val = 1 / (1 + std::exp(exp));
//     return val;
// }

// int cMathUtil::SampleDiscreteProb(const tVectorX &probs)
// {
//     assert(std::abs(probs.sum() - 1) < 0.00001);
//     FLOAT rand = RandFloat();

//     int rand_idx = gInvalidIdx;
//     int num_probs = static_cast<int>(probs.size());
//     for (int i = 0; i < num_probs; ++i)
//     {
//         FLOAT curr_prob = probs[i];
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
int cMathUtil::RandIntCategorical(const std::vector<FLOAT> &prop)
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

    FLOAT d00 = v0.dot(v0);
    FLOAT d01 = v0.dot(v1);
    FLOAT d11 = v1.dot(v1);
    FLOAT d20 = v2.dot(v0);
    FLOAT d21 = v2.dot(v1);
    FLOAT denom = d00 * d11 - d01 * d01;
    FLOAT v = (d11 * d20 - d01 * d21) / denom;
    FLOAT w = (d00 * d21 - d01 * d20) / denom;
    FLOAT u = 1.0f - v - w;
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

bool cMathUtil::CheckNextInterval(FLOAT delta, FLOAT curr_val,
                                  FLOAT int_size)
{
    FLOAT pad = 0.001 * delta;
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
    FLOAT t = RandFloat(0, 1);
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
//	FLOAT eta = 0;
//	FLOAT q1, q2, q3, q4;	// = [w, x, y, z]
//
//	// determine q1
//	{
//		FLOAT detect_value = mat(0, 0) + mat(1, 1) + mat(2, 2);
//		if (detect_value > eta)
//		{
//			q1 = 0.5 * std::sqrt(1 + detect_value);
//		}
//		else
//		{
//			FLOAT numerator = 0;
//			numerator += std::pow(mat(2, 1) - mat(1, 2), 2);
//			numerator += std::pow(mat(0, 2) - mat(2, 0), 2);
//			numerator += std::pow(mat(1, 0) - mat(0, 1), 2);
//			q1 = 0.5 *  std::sqrt(numerator / (3 - detect_value));
//		}
//	}
//
//	// determine q2
//	{
//		FLOAT detect_value = mat(0, 0) - mat(1, 1) - mat(2, 2);
//		if (detect_value > eta)
//		{
//			q2 = 0.5 * std::sqrt(1 + detect_value);
//		}
//		else
//		{
//			FLOAT numerator = 0;
//			numerator += std::pow(mat(2, 1) - mat(1, 2), 2);
//			numerator += std::pow(mat(0, 1) + mat(1, 0), 2);
//			numerator += std::pow(mat(2, 0) + mat(0, 2), 2);
//			q2 = 0.5 * std::sqrt(numerator / (3 - detect_value));
//		}
//	}
//
//	// determine q3
//	{
//		FLOAT detect_value = -mat(0, 0) + mat(1, 1) - mat(2, 2);
//		if (detect_value > eta)
//		{
//			q3 = 0.5 * std::sqrt(1 + detect_value);
//		}
//		else
//		{
//			FLOAT numerator = 0;
//			numerator += std::pow(mat(0, 2) - mat(2, 0), 2);
//			numerator += std::pow(mat(0, 1) + mat(1, 0), 2);
//			numerator += std::pow(mat(1, 2) + mat(2, 1), 2);
//			q3 = 0.5 * std::sqrt(numerator / (3 - detect_value));
//		}
//	}
//
//	// determine q4
//	{
//		FLOAT detect_value = -mat(0, 0) - mat(1, 1) + mat(2, 2);
//		if (detect_value > eta)
//		{
//			q4 = 0.5 * std::sqrt(1 + detect_value);
//		}
//		else
//		{
//			FLOAT numerator = 0;
//			numerator += std::pow(mat(1, 0) - mat(0, 1), 2);
//			numerator += std::pow(mat(2, 0) + mat(0, 2), 2);
//			numerator += std::pow(mat(2, 1) + mat(1, 2), 2);
//			q4 = 0.5 * std::sqrt(numerator / (3 - detect_value));
//		}
//	}
//
//	return tQuaterniondd(q1, q2, q3, q4);
//}
