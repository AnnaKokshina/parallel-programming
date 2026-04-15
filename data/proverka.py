import numpy as np
import sys


def read_matrix(A, B, C):
    with open(A, "r") as f:
        # Читаем размерность
        n = int(f.readline().strip())

        # Читаем матрицу
        m1 = []
        for _ in range(n):
            row = list(map(int, f.readline().strip().split()))
            m1.append(row)

    with open(B, "r") as f:
        # Читаем размерность
        n = int(f.readline().strip())

        # Читаем матрицу
        m2 = []
        for _ in range(n):
            row = list(map(int, f.readline().strip().split()))
            m2.append(row)
    with open(C, "r") as f:
        # Читаем размерность
        n = int(f.readline().strip())

        # Читаем матрицу
        m3 = []
        for _ in range(n):
            row = list(map(int, f.readline().strip().split()))
            m3.append(row)

    return (
        np.array(m1, dtype=np.int64),
        np.array(m2, dtype=np.int64),
        np.array(m3, dtype=np.int64),
    )

def main():
    if len(sys.argv) != 4:
        sys.exit(-1)

    A = sys.argv[1]
    B = sys.argv[2]
    C = sys.argv[3]

    try:
        m1, m2, m3 = read_matrix(A, B, C)

        n = m1.shape[0]

        if (
            m1.shape != (n, n)
            or m2.shape != (n, n)
            or m3.shape != (n, n)
        ):
            sys.exit(-1)

        res = np.dot(m1, m2)

        if np.array_equal(res, m3):
            sys.exit(0)
        else:
            sys.exit(1)

    except Exception as e:
        sys.exit(-1)


if __name__ == "__main__":
    main()
