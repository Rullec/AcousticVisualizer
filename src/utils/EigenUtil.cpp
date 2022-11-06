#include "utils/EigenUtil.h"

void removeRow(tMatrixX &matrix, unsigned int rowToRemove)
{
    unsigned int numRows = matrix.rows() - 1;
    unsigned int numCols = matrix.cols();

    if (rowToRemove < numRows)
        matrix.block(rowToRemove, 0, numRows - rowToRemove, numCols) =
            matrix.bottomRows(numRows - rowToRemove);

    matrix.conservativeResize(numRows, numCols);
}

void removeRowVec(tVectorX &matrix, unsigned int rowToRemove)
{
    unsigned int numRows = matrix.rows() - 1;

    if (rowToRemove < numRows)
        matrix.segment(rowToRemove, numRows - rowToRemove) =
            matrix.bottomRows(numRows - rowToRemove);

    matrix.conservativeResize(numRows);
}

void removeColumn(tMatrixX &matrix, unsigned int colToRemove)
{
    unsigned int numRows = matrix.rows();
    unsigned int numCols = matrix.cols() - 1;

    if (colToRemove < numCols)
        matrix.block(0, colToRemove, numRows, numCols - colToRemove) =
            matrix.rightCols(numCols - colToRemove);

    matrix.conservativeResize(numRows, numCols);
}

#include <algorithm>

void cEigenUtil::RemoveRowsAndColumns(tMatrixX &mat,
                                      std::vector<int> row_indices,
                                      std::vector<int> col_indices)
{
    // 1. sort indices from low to high

    std::sort(row_indices.begin(), row_indices.end());
    std::sort(col_indices.begin(), col_indices.end());
    // 2. delete them one by one
    for (int _idx = 0; _idx < row_indices.size(); _idx++)
    {
        int remove_idx = row_indices[_idx] - _idx;
        // printf("remove origin %d real %d\n", row_indices[_idx], remove_idx);
        removeRow(mat, remove_idx);
    }
    for (int _idx = 0; _idx < col_indices.size(); _idx++)
    {
        int remove_idx = col_indices[_idx] - _idx;
        // printf("remove origin %d real %d\n", col_indices[_idx], remove_idx);
        removeColumn(mat, remove_idx);
    }
}

void cEigenUtil::RemoveRows(tVectorX &vec, std::vector<int> remove_indices)
{
    std::sort(remove_indices.begin(), remove_indices.end());

    for (int _idx = 0; _idx < remove_indices.size(); _idx++)
    {
        int remove_idx = remove_indices[_idx] - _idx;
        removeRowVec(vec, remove_idx);
    }
}