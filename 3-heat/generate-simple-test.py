import typing as tp

from copy import deepcopy
from math import pi, exp, sin

DecGenerator = tp.Generator[float, None, None]

ROD_LENGTH = 1.0
ROD_PIECES = 51
SPACE_STEP = ROD_LENGTH / (ROD_PIECES - 1)

HEAT_INIT = 1
THERMAL_DIFFUSIVITY = 1.0

TIME_STEP = 0.0002
TIME_BEGIN = 0.0
TIME_END = 0.1


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


def main() -> None:
    rod = [4.0 * HEAT_INIT / pi * compute_row(SPACE_STEP * i, TIME_END)
           for i in range(ROD_PIECES)]
    print(*rod)


if __name__ == "__main__":
    main()
