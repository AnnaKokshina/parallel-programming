import numpy as np


def generate_matrix():
    sizes = [250]
    np.random.seed(52)

    for n in sizes:
        A = np.random.randint(-100, 101, size=(n, n))
        B = np.random.randint(-100, 101, size=(n, n))

        with open(f"A.txt", "w") as f:
            f.write(f"{n}\n")
            for i in range(n):
                f.write(" ".join(str(x) for x in A[i]) + "\n")
            f.write("\n")
        with open("B.txt", "w") as f:
            f.write(f"{n}\n")
            for i in range(n):
                f.write(" ".join(str(x) for x in B[i]) + "\n")
            f.write("\n")

        print(f"Generated")


if __name__ == "__main__":
    generate_matrix()