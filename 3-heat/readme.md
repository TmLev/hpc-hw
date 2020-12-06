# Heat

## Problem

Problem statement can be found [here](problem.pdf)
(English version is not available at the time of writing).

## Algorithm

Distributed approximate computation gives quite accurate solution – difference
with the exact solution is roughly `0.000010874`.

## Implementation details

* Every process shares his first cell (except for the first process)
  and his last cell (except for the last process) with the previous/next
  (w.r.t. rank) neighbour respectively using `MPI_Send/MPI_Recv`.
* The first process collects data from other processes in the end using
  `MPI_Gatherv`.
* Source code is located in [`src`](src) directory.

## Benchmarks

Measurements were taken on 3,1 GHz Dual-Core Intel Core i5
(2 cores, 4 threads with hyper-threading enabled) in
[`Release`](https://cmake.org/cmake/help/v3.18/variable/CMAKE_BUILD_TYPE.html)
build.

## References

* https://www.mpich.org/static/docs/v3.3/

## Appendix

Contact the author if Jupyter notebooks with code for plots are needed.
