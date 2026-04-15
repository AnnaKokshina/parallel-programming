#include "matrix.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <mpi.h>

using namespace std;

void save_mat(const string& filename, Matrix<int>& res) {
    ofstream f;
    f.open(filename);
    f << res.cols() << "\n" << res;
    f.close();
}

void save_res(const string& filename, Info res) {
    ofstream f;
    f.open(filename, ios::app);
    f << res;
    f.close();
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    size_t n = 0;
    vector<int> el_a, el_b;

    if (rank == 0) {
        ifstream fa("A.txt");
        if (!fa.is_open()) {
            cerr << "Failed to open A.txt" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fa >> n;
        el_a.resize(n * n);
        for (size_t i = 0; i < n * n; ++i) {
            fa >> el_a[i];
        }
        fa.close();

        ifstream fb("B.txt");
        if (!fb.is_open()) {
            cerr << "Failed to open B.txt" << endl;
            MPI_Abort(MPI_COMM_WORLD, 2);
        }
        size_t n_b;
        fb >> n_b;
        if (n_b != n) {
            cerr << "Matrix dimensions mismatch" << endl;
            MPI_Abort(MPI_COMM_WORLD, 3);
        }
        el_b.resize(n * n);
        for (size_t i = 0; i < n * n; ++i) {
            fb >> el_b[i];
        }
        fb.close();
    }

    MPI_Bcast(&n, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
    if (n == 0) {
        if (rank == 0) cerr << "Matrix size is zero" << endl;
        MPI_Finalize();
        return 1;
    }

    if (rank != 0) {
        el_a.resize(n * n);
        el_b.resize(n * n);
    }

    MPI_Bcast(el_a.data(), static_cast<int>(n * n), MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(el_b.data(), static_cast<int>(n * n), MPI_INT, 0, MPI_COMM_WORLD);

    Matrix<int> a(n, n, el_a.data());
    Matrix<int> b(n, n, el_b.data());

    auto res = multiply_matrix(a, b);

    if (rank == 0) {
        save_mat("Res.txt", res.matrix);

        int status = system("python proverka.py A.txt B.txt Res.txt");
        res.is_correct = (status == 0);
        if (status == -1) {
            cout << "Error in check result\n";
        }

        save_res("result.txt", res);
        res.graphic();

        cout << "Done with " << size << " processes\n";
    }

    MPI_Finalize();
    return 0;
}
