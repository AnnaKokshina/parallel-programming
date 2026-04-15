#ifndef MATRIX
#define MATRIX

#include <iostream>
#include <fstream>
#include <exception>
#include <algorithm>
#include <chrono>
#include <mpi.h>

template <class T>
class Matrix {
    T* _elements = nullptr;
    size_t _rows = 0;
    size_t _cols = 0;
public:
    Matrix() = default;
    Matrix(size_t rows, size_t cols) : _rows(rows), _cols(cols) {
        if (_rows * _cols == 0) {
            throw std::invalid_argument("Size of matrix = 0");
        }
        _elements = new T[rows * cols];
        for (size_t i = 0; i < rows * cols; i++) {
            _elements[i] = T(0);
        }
    }
    Matrix(size_t rows, size_t cols, T* elements) : _rows(rows), _cols(cols) {
        if (_rows * _cols == 0) {
            throw std::invalid_argument("Size of matrix = 0");
        }
        _elements = new T[rows * cols];
        for (size_t i = 0; i < rows * cols; i++) {
            _elements[i] = elements[i];
        }
    }
    Matrix(const Matrix<T>& other) : _rows(other._rows), _cols(other._cols) {
        if (_rows * _cols == 0) {
            throw std::invalid_argument("Size of matrix = 0");
        }
        _elements = new T[_rows * _cols];
        for (size_t i = 0; i < _rows * _cols; i++) {
            _elements[i] = other._elements[i];
        }
    }

    ~Matrix() {
        delete[] _elements;
    }

    Matrix<T>& operator=(const Matrix<T>& other) {
        if (this != &other) {
            delete[] _elements;
            _rows = other._rows;
            _cols = other._cols;
            _elements = new T[_rows * _cols];
            for (size_t i = 0; i < _rows * _cols; i++) {
                _elements[i] = other._elements[i];
            }
        }
        return *this;
    }

    size_t rows() const { return _rows; }
    size_t cols() const { return _cols; }

    T operator()(size_t rows, size_t cols) const {
        if (rows >= _rows || cols >= _cols) {
            throw std::invalid_argument("Out of range");
        }
        return _elements[rows * _cols + cols];
    }

    T& operator()(size_t rows, size_t cols) {
        if (rows >= _rows || cols >= _cols) {
            throw std::invalid_argument("Out of range");
        }
        return _elements[rows * _cols + cols];
    }

    Matrix<T>& operator*=(const Matrix<T>& rhs) {
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

    Matrix<T> operator*(const Matrix<T>& rhs) const {
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

struct Info {
    Matrix<int> matrix;
    std::chrono::milliseconds duration = std::chrono::milliseconds(0);
    bool is_correct = false;
    int np = 1;
    Info() = default;
    void graphic() {
        std::ofstream fout;
        fout.open("graphic.txt", std::ios::app);
        if (!fout.is_open()) {
            throw std::exception("Failed to save result");
        }
        fout << matrix.cols() << " " << duration.count() << " " << np << "\n";
        fout.close();
    }
};

std::ostream& operator<<(std::ostream& os, const Info& s) {
    os << "Duration: " << s.duration.count() << " ms\n";
    os << "Is correct: " << s.is_correct << "\n";
    os << "Processes: " << s.np << "\n";
    os << "Rows: " << s.matrix.rows() << "\n\n";
    return os;
}

Info multiply_matrix(Matrix<int>& a, Matrix<int>& b) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (a.cols() != b.cols() || a.rows() != a.cols() || b.rows() != b.cols()) {
        if (rank == 0) {
            throw std::invalid_argument("Matrix must be N*N!");
        }
    }

    if (rank == 0) {
        std::cout << "Start MPI multiplication with " << size << " processes\n";
    }

    double start_time = MPI_Wtime();

    size_t N = a.rows();
    size_t base_rows = N / size;
    size_t remainder = N % size;
    size_t local_rows = (rank < static_cast<int>(remainder)) ? base_rows + 1 : base_rows;
    size_t start_row = (rank < static_cast<int>(remainder))
        ? rank * (base_rows + 1)
        : remainder * (base_rows + 1) + (rank - remainder) * base_rows;

    std::vector<int> local_result(local_rows * N, 0);

    for (size_t i = 0; i < local_rows; ++i) {
        size_t global_i = start_row + i;
        for (size_t j = 0; j < N; ++j) {
            int sum = 0;
            for (size_t k = 0; k < N; ++k) {
                sum += a(global_i, k) * b(k, j);
            }
            local_result[i * N + j] = sum;
        }
    }

    std::vector<int> recv_counts(size);
    std::vector<int> displs(size);
    if (rank == 0) {
        for (int i = 0; i < size; ++i) {
            size_t rows_i = (i < static_cast<int>(remainder)) ? base_rows + 1 : base_rows;
            recv_counts[i] = static_cast<int>(rows_i * N);
            displs[i] = (i == 0) ? 0 : displs[i - 1] + recv_counts[i - 1];
        }
    }

    std::vector<int> full_result;
    if (rank == 0) {
        full_result.resize(N * N);
    }

    MPI_Gatherv(local_result.data(), static_cast<int>(local_rows * N), MPI_INT,
        full_result.data(), recv_counts.data(), displs.data(), MPI_INT,
        0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    Info res;
    if (rank == 0) {
        Matrix<int> result(N, N);
        for (size_t i = 0; i < N * N; ++i) {
            result(i / N, i % N) = full_result[i];
        }
        res.matrix = result;
        res.duration = std::chrono::milliseconds(static_cast<long long>((end_time - start_time) * 1000));
        res.np = size;
        std::cout << "Finish multiply\n";
    }

    return res;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& Matrix) {
    for (size_t i = 0; i < Matrix.rows(); i++) {
        for (size_t j = 0; j < Matrix.cols(); j++) {
            os << Matrix(i, j) << " ";
        }
        os << "\n";
    }
    return os;
}

#endif
