#ifndef MATRIX
#define MATRIX


#include <iostream>
#include <exception>
#include <algorithm>
#include <chrono>

template <class T>
class Matrix {
	T* _elements;
	size_t _rows;
	size_t _cols;
public:
    Matrix(size_t rows, size_t cols) : _rows(rows), _cols(cols) {
        if (_rows * _cols == 0)
        {
            throw std::invalid_argument("Size of matrix = 0");
        }
        _elements = new T[rows * cols];
        for (size_t i = 0; i < rows * cols; i++) {
            _elements[i] = T(0);
        }
    }
	Matrix(size_t rows, size_t cols, T* elements) : _rows(rows), _cols(cols) {
		if (_rows * _cols == 0)
		{
			throw std::invalid_argument("Size of matrix = 0");
		}
		_elements = new T[rows * cols];
		for (size_t i = 0; i < rows * cols; i++) {
			_elements[i] = elements[i];
		}
	}
	Matrix(const Matrix<T>& other) : _rows(other._rows), _cols(other._cols)
	{
		if (_rows * _cols == 0)
		{
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

	size_t rows() const
	{
		return _rows;
	}
	size_t cols() const
	{
		return _cols;
	}
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

template <typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& Matrix)
{
    for (size_t i = 0; i < Matrix.rows(); i++) {
        for (size_t j = 0; j < Matrix.cols(); j++) {
            os << Matrix(i, j) << " ";
        }
        os << "\n";
    }
    return os;
}

#endif
