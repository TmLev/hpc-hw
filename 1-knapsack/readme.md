# Knapsack


## Benchmarks
* The algorithm performs rather uniform on small tests no matter how many threads are running.
  
  ![small-absolute](plots/small-tests-absolute.png)

* However, there is a remarkable difference for medium tests. 
  As can be seen below, single thread requires too much time on tests 6 and 10 to be compared against multithreaded run.
  
  ![medium-absolute](plots/medium-tests-absolute.png)

* For clarity, here is the same plot as above but without single thread measurements.
  
  ![medium-absolute-without-one](plots/medium-tests-absolute-without-one.png)

## References
* https://www0.gsb.columbia.edu/mygsb/faculty/research/pubfiles/4407/kolesar_branch_bound.pdf

## Appendix
Contact the author if Jupyter notebooks with code for plots are needed.
