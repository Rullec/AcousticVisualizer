#ifndef EIGEN_UTIL_H_
#define EIGEN_UTIL_H_
#include "utils/BaseTypeUtil.h"
#include <Eigen/Dense>
#include <vector>
typedef Eigen::VectorXi tVectorXi;
typedef Eigen::Vector3i tVector3i;
typedef Eigen::Vector4i tVector4i;
typedef Eigen::Matrix<FLOAT, Eigen::Dynamic, 1> tVectorX;
typedef Eigen::Matrix<float, Eigen::Dynamic, 1> tVectorXf;
typedef Eigen::Matrix<FLOAT, 3, 1> tVector3;
typedef Eigen::Matrix<FLOAT, 4, 1> tVector4;
typedef Eigen::Matrix<FLOAT, 2, 1> tVector2;
typedef Eigen::Vector2i tVector2i;
typedef Eigen::Matrix2i tMatrix2i;
typedef Eigen::Matrix<FLOAT, 2, 2> tMatrix2;
typedef Eigen::Matrix<FLOAT, 3, 3> tMatrix3;
typedef Eigen::Matrix<FLOAT, 4, 4> tMatrix4;
typedef Eigen::Matrix<FLOAT, Eigen::Dynamic, Eigen::Dynamic> tMatrixX;
typedef Eigen::MatrixXi tMatrixXi;
typedef Eigen::Quaternion<FLOAT> tQuaternion;

template <typename T>
using tEigenArr = std::vector<T, Eigen::aligned_allocator<T>>;
typedef tEigenArr<tVector4> tVectorArr;

class cEigenUtil
{
public:
    static void RemoveRowsAndColumns(tMatrixX &mat,
                                     std::vector<int> row_indices,
                                     std::vector<int> col_indices);
    static void RemoveRows(tVectorX &mat, std::vector<int> row_indices);
};

#endif