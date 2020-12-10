import subprocess
import typing as tp

from copy import deepcopy
from math import pi, exp, sin

DecGenerator = tp.Generator[float, None, None]
Heat = tp.List[float]

PROCESSES = 4

ROD_LENGTH = 1.0  # l
ROD_PIECES = 51
SPACE_STEP = ROD_LENGTH / (ROD_PIECES - 1)  # h

HEAT_INIT = 1  # u_0
THERMAL_DIFFUSIVITY = 1.0  # k

TIME_STEP = 0.0002  # dt
TIME_BEGIN = 0.0  # t_0
TIME_END = 0.1  # T


def decrange(start: float, stop: float, step: float) -> DecGenerator:
    while start < stop:
        yield start
        start += step


def compute_row(x: float, t: float) -> float:
    sum_ = 0.0

    for m in range(1_000_000):
        old = deepcopy(sum_)

        ind = 2 * m + 1
        sum_ += exp(
            -THERMAL_DIFFUSIVITY
            * (pi ** 2) * (ind ** 2)
            * t / (ROD_LENGTH ** 2)
        ) / ind * sin(pi * ind * x / ROD_LENGTH)

        if abs(old - sum_) < 0.000001:
            break

    return sum_


def get_mpi_answer() -> Heat:
    completed = subprocess.run(["./run-mpi.sh", str(PROCESSES)],
                               capture_output=True)
    _, heat, *_ = completed.stdout.decode().split("\n")
    return list(map(float, heat.split()))


def get_exact_answer() -> Heat:
    heat = [4.0 * HEAT_INIT / pi * compute_row(SPACE_STEP * i, TIME_END)
            for i in range(ROD_PIECES)]
    round_ = lambda h: round(h, 7)
    return list(map(round_, heat))


def compute_errors(mpi: Heat, exact: Heat) -> Heat:
    return [abs(mpi[i] - exact[i]) for i in range(len(mpi))]


def main() -> None:
    mpi = get_mpi_answer()[0::5]
    print("MPI program solution:", mpi, sep="\n", end="\n\n")

    exact = get_exact_answer()[0::5]
    print("Exact solution:", exact, sep="\n", end="\n\n")

    errors = compute_errors(mpi, exact)
    print("Per point errors for 11 points (0.0, 0.1, ..., 1.0):",
          errors, sep="\n", end="\n\n")

    print(f"Mean error: {sum(errors) / len(errors)}")


if __name__ == "__main__":
    main()
