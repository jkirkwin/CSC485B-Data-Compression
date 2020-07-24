#ifndef UVG_MATRIX_H
#define UVG_MATRIX_H

#include <vector>
#include <cassert>

namespace matrix {

    // todo parameterize on weight and height too so we can use arrays?
    template <class T>
    struct Matrix {
        const unsigned int rows, cols;
        std::vector<T> data;

        unsigned long capacity() const {
            return rows*cols;
        }

        T at(unsigned int row, unsigned int col) const {
            return data.at(row*cols + col);
        }

        void set(unsigned int row, unsigned int col, T value) {
            data.at(row*cols + col) = value;
        }

        void insertBlock(unsigned int startRow, unsigned int startCol, const Matrix<T>& block) {
            assert (startRow + block.rows <= this->rows);
            assert (startCol + block.cols <= this->cols);

            for (int i = 0; i < block.rows; ++i) {
                for (int j = 0; j < block.cols; ++j) {
                    auto val = block.at(i, j);
                    set(startRow + i, startCol + j, val);
                }
            }
        }

        Matrix<T> getBlock(
                unsigned int startRow,
                unsigned int startCol,
                unsigned int numRows,
                unsigned int numCols) {
            assert (startRow + numRows <= this->rows);
            assert (startCol + numCols <= this->cols);

            Matrix<T> block(numRows, numCols);
            for (int i = 0; i < block.rows; ++i) {
                for (int j = 0; j < block.cols; ++j) {
                    auto val = this->at(startRow + i, startCol + j);
                    block.set(i, j, val);
                }
            }
            return block;
        }

        Matrix(unsigned int rows, unsigned int cols): rows(rows), cols(cols), data(rows*cols) {
            assert (rows > 0 && cols > 0);
        }

        Matrix(unsigned int rows, unsigned int cols, T value): rows(rows), cols(cols), data(rows*cols, value) {
            assert (rows > 0 && cols > 0);
        }

        Matrix(unsigned int rows, unsigned int cols, std::vector<T> data): rows(rows), cols(cols), data(data) {
            assert (rows > 0 && cols > 0);
            assert (data.size() == rows * cols);
        }
    };

    template<typename T>
    Matrix<T> transpose(const Matrix<T>& m) {
        Matrix<T> mTranspose(m.cols, m.rows);
        for (int i = 0; i < m.rows; ++i) {
            for (int j = 0; j < m.rows; ++j) {
                mTranspose.set(i, j, m.at(j, i));
            }
        }
        return mTranspose;
    }

    template<typename T>
    Matrix<T> multiply(const Matrix<T>& lhs, const Matrix<T>& rhs) {
        assert (lhs.cols == rhs.rows);
        auto resultRows = lhs.rows;
        auto resultCols = rhs.cols;
        Matrix<T> result(resultRows, resultCols);

        for (int i = 0; i < resultRows; ++i) {
            for (int j = 0; j < resultCols; ++j) {
                auto sum = 0;
                for (int k = 0; k < lhs.cols; ++k) {
                    sum += lhs.at(i, k) * rhs.at(k, j);
                }
                result.set(i, j, sum);
            }
        }
        return result;
    }
}

#endif //UVG_MATRIX_H
