#pragma once
#include "EigenUtil.h"
#include "Rand.h"
#include <random>
#include "BaseTypeUtil.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

const int gInvalidIdx = -1;

// for convenience define standard vector for rendering

const _FLOAT gRadiansToDegrees = 57.2957795;
const _FLOAT gDegreesToRadians = 1.0 / gRadiansToDegrees;
const _FLOAT gInchesToMeters = 0.0254;
const _FLOAT gFeetToMeters = 0.3048;

class cMathUtil
{
public:
    static bool IsPoint(const tVector4 &vec);
    static tVector4 VecToPoint(const tVector4 &vec);
    static int Clamp(int val, int min, int max);
    // static void Clamp(const tVectorXd &min, const tVectorXd &max,
    //                   tVectorXd &out_vec);
    static _FLOAT Clamp(_FLOAT val, _FLOAT min, _FLOAT max);
    static _FLOAT Saturate(_FLOAT val);
    static _FLOAT Lerp(_FLOAT t, _FLOAT val0, _FLOAT val1);

    static _FLOAT NormalizeAngle(_FLOAT theta);

    // rand number
    static _FLOAT RandFloat();
    static _FLOAT RandFloat(_FLOAT min, _FLOAT max);
    static _FLOAT RandFloatNorm(_FLOAT mean, _FLOAT stdev);
    static _FLOAT RandFloatExp(_FLOAT lambda);
    static _FLOAT RandFloatSeed(_FLOAT seed);
    static int RandInt();
    static int RandInt(int min, int max);
    static int RandUint();
    static int RandUint(unsigned int min, unsigned int max);
    static int RandIntExclude(int min, int max, int exc);
    static void SeedRand(unsigned long int seed);
    static int RandSign();
    static bool FlipCoin(_FLOAT p = 0.5);
    static _FLOAT SmoothStep(_FLOAT t);

    static _FLOAT Sign(_FLOAT val);
    static int Sign(int val);

    static _FLOAT AddAverage(_FLOAT avg0, int count0, _FLOAT avg1, int count1);
    static tVector4 AddAverage(const tVector4 &avg0, int count0,
                              const tVector4 &avg1, int count1);
    // static void AddAverage(const tVectorXd &avg0, int count0,
    //                        const tVectorXd &avg1, int count1,
    //                        tVectorXd &out_result);
    // static void CalcSoftmax(const tVectorXd &vals, _FLOAT temp,
    //                         tVectorXd &out_prob);
    // static _FLOAT EvalGaussian(const tVectorXd &mean,
    //                            const tVectorXd &covar,
    //                            const tVectorXd &sample);
    // static _FLOAT EvalGaussian(_FLOAT mean, _FLOAT covar, _FLOAT sample);
    // static _FLOAT CalcGaussianPartition(const tVectorXd &covar);
    // static _FLOAT EvalGaussianLogp(_FLOAT mean, _FLOAT covar, _FLOAT sample);
    // static _FLOAT EvalGaussianLogp(const tVectorXd &mean,
    //                                const tVectorXd &covar,
    //                                const tVectorXd &sample);
    // static _FLOAT Sigmoid(_FLOAT x);
    // static _FLOAT Sigmoid(_FLOAT x, _FLOAT gamma, _FLOAT bias);

    // static int SampleDiscreteProb(const tVectorXd &probs);
    static tVector4 CalcBarycentric(const tVector4 &p, const tVector4 &a,
                                   const tVector4 &b, const tVector4 &c);

    static bool ContainsAABB(const tVector4 &pt, const tVector4 &aabb_min,
                             const tVector4 &aabb_max);
    static bool ContainsAABB(const tVector4 &aabb_min0, const tVector4 &aabb_max0,
                             const tVector4 &aabb_min1,
                             const tVector4 &aabb_max1);
    static bool ContainsAABBXZ(const tVector4 &pt, const tVector4 &aabb_min,
                               const tVector4 &aabb_max);
    static bool ContainsAABBXZ(const tVector4 &aabb_min0,
                               const tVector4 &aabb_max0,
                               const tVector4 &aabb_min1,
                               const tVector4 &aabb_max1);
    static void CalcAABBIntersection(const tVector4 &aabb_min0,
                                     const tVector4 &aabb_max0,
                                     const tVector4 &aabb_min1,
                                     const tVector4 &aabb_max1, tVector4 &out_min,
                                     tVector4 &out_max);
    static void CalcAABBUnion(const tVector4 &aabb_min0,
                              const tVector4 &aabb_max0,
                              const tVector4 &aabb_min1,
                              const tVector4 &aabb_max1, tVector4 &out_min,
                              tVector4 &out_max);
    static bool IntersectAABB(const tVector4 &aabb_min0,
                              const tVector4 &aabb_max0,
                              const tVector4 &aabb_min1,
                              const tVector4 &aabb_max1);
    static bool IntersectAABBXZ(const tVector4 &aabb_min0,
                                const tVector4 &aabb_max0,
                                const tVector4 &aabb_min1,
                                const tVector4 &aabb_max1);

    // check if curr_val and curr_val - delta belong to different intervals
    static bool CheckNextInterval(_FLOAT delta, _FLOAT curr_val,
                                  _FLOAT int_size);

    static tVector4 SampleRandPt(const tVector4 &bound_min,
                                const tVector4 &bound_max);
    // samples a bound within the given bounds with a benter towards the focus
    // pt
    static tVector4 SampleRandPtBias(const tVector4 &bound_min,
                                    const tVector4 &bound_max);
    static tVector4 SampleRandPtBias(const tVector4 &bound_min,
                                    const tVector4 &bound_max,
                                    const tVector4 &focus);

    static tMatrix4 VectorToSkewMat(const tVector4 &);
    static tMatrix3 VectorToSkewMat(const tVector3 &);
    static tVector4 SkewMatToVector(const tMatrix4 &);
    static bool IsSame(const tVector4 &v1, const tVector4 &v2, const _FLOAT eps);
    static void ThresholdOp(tVectorX &v, _FLOAT threshold = 1e-6);
    static tVector4 CalcAxisAngleFromOneVectorToAnother(const tVector4 &v0,
                                                       const tVector4 &v1);
    template <typename T> static const std::string EigenToString(const T &mat)
    {
        std::stringstream ss;
        ss << mat;
        return ss.str();
    }
    static _FLOAT Truncate(_FLOAT num, int digits = 5);
    static tMatrixX ExpandFrictionCone(int num_friction_dirs,
                                        const tVector3 &normal);
    static tMatrix4 InverseTransform(const tMatrix4 &);
    static _FLOAT CalcConditionNumber(const tMatrixX &mat);
    // static tMatrixX JacobPreconditioner(const tMatrixX &mat);
    // static void RoundZero(tMatrixX &mat, _FLOAT threshold = 1e-10);

    template <typename T>
    static void RoundZero(T &mat, _FLOAT threshold = 1e-10)
    {
        mat = (threshold < mat.array().abs()).select(mat, 0.0f);
    }
    template <typename T> static tVector4 Expand(const T &vec, _FLOAT n)
    {
        return tVector4(vec[0], vec[1], vec[2], n);
    }
    template <typename T> static tMatrix4 ExpandMat(const T &raw_mat)
    {
        tMatrix4 mat = tMatrix4::Zero();
        mat.block(0, 0, 3, 3) = raw_mat.block(0, 0, 3, 3);
        return mat;
    }
    static tVector4 RayCastTri(const tVector4 &ori, const tVector4 &dir,
                              const tVector4 &p1, const tVector4 &p2,
                              const tVector4 &p3, _FLOAT eps = 1e-10);
    static tVector4 RayCastPlane(const tVector4 &ray_ori, const tVector4 &ray_dir,
                                const tVector4 &plane_eqaution,
                                _FLOAT eps = 1e-10);
    static tMatrixX
    CartesianProduct(const std::vector<std::vector<_FLOAT>> &lists);
    static std::vector<std::vector<_FLOAT>>
    CartesianProductVec(const std::vector<std::vector<_FLOAT>> &lists);
    static _FLOAT CalcDistanceFromPointToLine(const tVector3 &point,
                                              const tVector3 &line_origin,
                                              const tVector3 &line_end);
    static tVector4 CalcNormalFromPlane(const tVector4 &plane_equation);
    static _FLOAT EvaluatePlane(const tVector4 &plane, const tVector4 &point);
    static _FLOAT CalcPlanePointDist(const tVector4 &plane,
                                     const tVector3 &point);
    static tVector4 SampleFromPlane(const tVector4 &plane_equation);
    static _FLOAT CalcTriangleArea(const tVector4 &p0, const tVector4 &p1,
                                  const tVector4 &p2);
    static _FLOAT CalcTriangleArea3d(const tVector3 &p0, const tVector3 &p1,
                                    const tVector3 &p2);
    static int RandIntCategorical(const std::vector<_FLOAT> &prop);
    template <typename dtype, int N>
    static int Argmax(const Eigen::Matrix<dtype, N, 1> &vec)
    {
        int arg_max = 0;
        dtype cur_max = vec[0];
        for (int i = 1; i < N; i++)
        {
            if (vec[i] > cur_max)
            {
                arg_max = i, cur_max = vec[i];
            }
        }
        return arg_max;
    }

private:
    static cRand gRand;

    template <typename T> static T SignAux(T val)
    {
        return (T(0) < val) - (val < T(0));
    }
};
