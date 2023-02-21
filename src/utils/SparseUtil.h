#include <Eigen/Sparse>
#include "utils/BaseTypeUtil.h"
typedef Eigen::SparseMatrix<_FLOAT, Eigen::RowMajor> tSparseMat;
typedef Eigen::SparseMatrix<_FLOAT, Eigen::ColMajor> tSparseColMat;
typedef Eigen::Triplet<_FLOAT> tTriplet;