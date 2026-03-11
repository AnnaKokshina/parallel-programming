#include "matrix.h"

using namespace std;

void save_mat(const string& filename, Matrix<int>& res)
{
    ofstream f;
    f.open(filename);
    f << res.cols() << "\n" << res;
    f.close();
}

void save_res(const string& filename, Info res)
{
    ofstream f;
    f.open(filename, ios::app);
    f << res;
    f.close();
}


int main() {
    ifstream fa;
    fa.open("A.txt");

    if (!fa.is_open())
    {
        throw std::exception("Failed open file");
    }

    size_t n = 0;
    fa >> n;
    int* el_a = new int[n * n];

    while (!fa.eof())
    {
        for (size_t i = 0; i < n * n; i++)
        {
            fa >> el_a[i];
        }

    }
    Matrix<int> a(n, n, el_a);
    delete[] el_a;
    fa.close();


    ifstream fb;
    fb.open("B.txt");

    if (!fb.is_open())
    {
        throw std::exception("Failed open file");
    }

    n = 0;
    fb >> n;
    int* el_b = new int[n * n];

    while (!fb.eof())
    {
        for (size_t i = 0; i < n * n; i++)
        {
            fb >> el_b[i];
        }

    }
    Matrix<int> b(n, n, el_b);
    delete[] el_b;

    fb.close();

    auto res = multiply_matrix(a, b);


    save_mat("Res.txt", res.matrix);


    int status = system("python proverka.py A.txt B.txt Res.txt");

    res.is_correct = !status;

    if (status == -1)
    {
        cout << "Error in check result";
    }

    save_res("result.txt", res);

    res.graphic();

    cout << "Done\n";

    return 0;

}
