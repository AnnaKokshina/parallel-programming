#include "matrix.h"
#include <vector>
#include <fstream>

using namespace std;

void save_mat(const string& filename, Matrix<int>& res) {
    ofstream f(filename);
    f << res.cols() << "\n" << res;
}

void save_res(const string& filename, Info res) {
    ofstream f(filename, ios::app);
    f << res;
}

int main() {
    ifstream fa("A.txt");
    if (!fa.is_open()) throw std::exception("Failed open A.txt");

    size_t n = 0;
    fa >> n;
    int* el_a = new int[n * n];
    while (!fa.eof()) {
        for (size_t i = 0; i < n * n; i++) fa >> el_a[i];
    }
    Matrix<int> a(n, n, el_a);
    delete[] el_a;
    fa.close();

    ifstream fb("B.txt");
    if (!fb.is_open()) throw std::exception("Failed open B.txt");

    n = 0;
    fb >> n;
    int* el_b = new int[n * n];
    while (!fb.eof()) {
        for (size_t i = 0; i < n * n; i++) fb >> el_b[i];
    }
    Matrix<int> b(n, n, el_b);
    delete[] el_b;
    fb.close();
    vector<int> block_sizes = { 8, 16, 32, 64 };

    ofstream research("cuda_research.csv");
    research << "matrix_size,block_size,time_ms,is_correct\n";

    for (int bs : block_sizes) {
        cout << "\n--- Testing block size = " << bs << " ---\n";
        auto res = multiply_matrix(a, b, bs);

        save_mat("Res.txt", res.matrix);

        int status = system("python proverka.py A.txt B.txt Res.txt");
        res.is_correct = (status == 0);
        if (status == -1)
            cout << "Error in check result\n";

        save_res("result.txt", res);
        res.graphic();
        research << n << "," << bs << "," << res.duration.count() << "," << res.is_correct << "\n";

        cout << "Done. Time = " << res.duration.count() << " ms, Correct = " << res.is_correct << "\n";
    }

    research.close();
    cout << "\nResearch results saved to cuda_research.csv\n";
    return 0;
}
