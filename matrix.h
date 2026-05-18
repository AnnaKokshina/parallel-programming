#ifndef MATRIX
#define MATRIX

#include <iostream>
#include <fstream>
#include <exception>
#include <algorithm>
#include <chrono>
#include <cuda_runtime.h>

struct Info {
    Matrix<int> matrix;
    std::chrono::milliseconds duration = std::chrono::milliseconds(0);
    bool is_correct = false;
    int block_size = 16;  
    Info() = default;

    void graphic() {
        std::ofstream fout;
        fout.open("graphic.txt", std::ios::app);
        if (!fout.is_open()) throw std::exception("Failed to save result");
        fout << matrix.cols() << " " << duration.count() << " " << block_size << "\n";
        fout.close();
    }
};

std::ostream& operator<<(std::ostream& os, const Info& s) {
    os << "Duration: " << s.duration.count() << " ms\n";
    os << "Is correct: " << s.is_correct << "\n";
    os << "Block size: " << s.block_size << "\n";
    os << "Rows: " << s.matrix.rows() << "\n\n";
    return os;
}

template <class T>
class Matrix {
    T* _elements = nullptr;
    size_t _rows = 0;
    size_t _cols = 0;
public:
    Matrix() = default;
    Matrix(size_t rows, size_t cols) : _rows(rows), _cols(cols) {
        if (_rows * _cols == 0) throw std::invalid_argument("Size of matrix = 0");
        _elements = new T[rows * cols]();
    }
    Matrix(size_t rows, size_t cols, T* elements) : _rows(rows), _cols(cols) {
        if (_rows * _cols == 0) throw std::invalid_argument("Size of matrix = 0");
        _elements = new T[rows * cols];
        for (size_t i = 0; i < rows * cols; i++) _elements[i] = elements[i];
    }
    Matrix(const Matrix<T>& other) : _rows(other._rows), _cols(other._cols) {
        if (_rows * _cols == 0) throw std::invalid_argument("Size of matrix = 0");
        _elements = new T[_rows * _cols];
        for (size_t i = 0; i < _rows * _cols; i++) _elements[i] = other._elements[i];
    }
    ~Matrix() { delete[] _elements; }

    Matrix<T>& operator=(const Matrix<T>& other) {
        if (this != &other) {
            delete[] _elements;
            _rows = other._rows;
            _cols = other._cols;
            _elements = new T[_rows * _cols];
            for (size_t i = 0; i < _rows * _cols; i++) _elements[i] = other._elements[i];
        }
        return *this;
    }

    size_t rows() const { return _rows; }
    size_t cols() const { return _cols; }

    T operator()(size_t rows, size_t cols) const {
        if (rows >= _rows || cols >= _cols) throw std::invalid_argument("Out of range");
        return _elements[rows * _cols + cols];
    }
    T& operator()(size_t rows, size_t cols) {
        if (rows >= _rows || cols >= _cols) throw std::invalid_argument("Out of range");
        return _elements[rows * _cols + cols];
    }

    Matrix<T>& operator*=(const Matrix<T>& rhs)
    {
        if (_cols != rhs._rows) {
            throw std::logic_error("the matrices are not consistent");
        }
        T* result_elements = new T[_rows * rhs._cols];
        for (size_t i = 0; i < _rows * rhs._cols; i++) {
            result_elements[i] = T(0);
        }
        for (size_t i = 0; i < _rows; i++) {
            for (size_t j = 0; j < rhs._cols; j++) {
                for (size_t k = 0; k < _cols; k++) {
                    result_elements[i * rhs._cols + j] +=
                        _elements[i * _cols + k] * rhs(k, j);
                }
            }
        }

        delete[] _elements;
        _elements = result_elements;
        _cols = rhs._cols;

        return *this;
    }

    Matrix<T> operator*(const Matrix<T>& rhs) const
    {
        if (_cols != rhs._rows) {
            throw std::logic_error("the matrices are not consistent");
        }

        Matrix<T> result(_rows, rhs._cols);

        for (size_t i = 0; i < _rows; i++) {
            for (size_t j = 0; j < rhs._cols; j++) {
                for (size_t k = 0; k < _cols; k++) {
                    result(i, j) += (*this)(i, k) * rhs(k, j);
                }
            }
        }

        return result;
    }
};


__global__ void matMulKernel(const int* A, const int* B, int* C, int N, int blockSize) {
    extern __shared__ int shared[];
    int* As = shared;
    int* Bs = &shared[blockSize * blockSize];

    int bx = blockIdx.x, by = blockIdx.y;
    int tx = threadIdx.x, ty = threadIdx.y;
    int row = by * blockSize + ty;
    int col = bx * blockSize + tx;
    int sum = 0;

    for (int k = 0; k < (N + blockSize - 1) / blockSize; ++k) {
        if (row < N && (k * blockSize + tx) < N)
            As[ty * blockSize + tx] = A[row * N + k * blockSize + tx];
        else
            As[ty * blockSize + tx] = 0;

        if (col < N && (k * blockSize + ty) < N)
            Bs[ty * blockSize + tx] = B[(k * blockSize + ty) * N + col];
        else
            Bs[ty * blockSize + tx] = 0;

        __syncthreads();

        for (int i = 0; i < blockSize; ++i)
            sum += As[ty * blockSize + i] * Bs[i * blockSize + tx];

        __syncthreads();
    }
    if (row < N && col < N)
        C[row * N + col] = sum;
}


Info multiply_matrix(Matrix<int>& a, Matrix<int>& b, int block_size) {
    if (a.rows() != a.cols() || b.rows() != b.cols() || a.cols() != b.rows())
        throw std::invalid_argument("Both matrices must be square and of the same size (N x N)");

    size_t N = a.rows();
    size_t size = N * N * sizeof(int);

    int* d_A, * d_B, * d_C;
    cudaMalloc(&d_A, size);
    cudaMalloc(&d_B, size);
    cudaMalloc(&d_C, size);

    cudaMemcpy(d_A, &a(0, 0), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, &b(0, 0), size, cudaMemcpyHostToDevice);


    dim3 blockDim(block_size, block_size);
    dim3 gridDim((N + block_size - 1) / block_size, (N + block_size - 1) / block_size);
    size_t sharedMemSize = 2 * block_size * block_size * sizeof(int);

    auto start = std::chrono::high_resolution_clock::now();

    matMulKernel << <gridDim, blockDim, sharedMemSize >> > (d_A, d_B, d_C, N, block_size);
    cudaDeviceSynchronize();

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    Matrix<int> result(N, N);
    cudaMemcpy(&result(0, 0), d_C, size, cudaMemcpyDeviceToHost);

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    Info res;
    res.matrix = result;
    res.duration = duration;
    res.block_size = block_size;
    return res;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& matrix) {
    for (size_t i = 0; i < matrix.rows(); ++i) {
        for (size_t j = 0; j < matrix.cols(); ++j)
            os << matrix(i, j) << " ";
        os << "\n";
    }
    return os;
}

#endif
