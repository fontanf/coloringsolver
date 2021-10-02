# Coloring Solver

A solver for the Graph Coloring Problem.

![graphcoloring](img/graphcoloring.png?raw=true "graphcoloring")

[image source](https://commons.wikimedia.org/wiki/File:Petersen_graph_3-coloring.svg)

## Implemented algorithms

* Greedy algorithms, see "ColPack: Software for graph coloring and related problems in scientific computing" (Gebremedhin et al., 2013) for their descriptions:
  * Largest first `-a greedy --ordering largestfirst`
  * Incidence degree `-a greedy --ordering incidencedegree --reverse`
  * Smallest last `-a greedy --ordering smallestlast --reverse`
  * Dynamic largest first `-a greedy --ordering dynamiclargestfirst`
  * DSATUR `-a greedy_dsatur`

* MILP (CPLEX), see "New Integer Linear Programming Models for the Vertex Coloring Problem" (Jabrayilov et Mutzel, 2018) for model descriptions:
  * Assignment-based ILP model `-a milp_assignment_cplex`
  * Representatives ILP model `-a milp_representatives_cplex`
  * Partial-ordering based ILP model `-a milp_partialordering_cplex`
  * Partial-ordering based ILP model 2 `-a milp_partialordering2_cplex`

* Local search algorithm implemented with [fontanf/localsearchsolver](https://github.com/fontanf/localsearchsolver) `-a "localsearch"`

* Row weighting local search
  * Penalize conflicting edges `-a "localsearch_rowweighting --iterations 100000 --iterations-without-improvment 10000"`
  * Penalize uncolored vertices `-a "localsearch_rowweighting_2 --iterations 100000 --iterations-without-improvment 10000"`

* Column generation heuristics implemented with [fontanf/columngenerationsolver](https://github.com/fontanf/columngenerationsolver):
  * Greedy `columngenerationheuristic_greedy`
  * Limited discrepancy search `columngenerationheuristic_limiteddiscrepancysearch`

## Usage (command line)

Download and uncompress the instances in the `data/` folder:

https://drive.google.com/file/d/1ZT2dSIleWN__MZIscsc3kAqGRVU91CDw/view?usp=sharing

Compile:
```shell
bazel build -- //...
```

Run:
```shell
./bazel-bin/coloringsolver/main -v -i "data/graphcoloring/1-FullIns_3.col" -a greedy_dsatur -c solution.txt
./bazel-bin/coloringsolver/main -v -i "data/graphcoloring/r1000.5.col" -a "localsearch_rowweighting --threads 3"
./bazel-bin/coloringsolver/main -v -i "data/graphcoloring/1-FullIns_3.col" -a milp_assignment_cplex -t 60 -c solution.txt
```

Benchmarks:
```shell
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy --ordering largestfirst"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy --ordering largestfirst --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy --ordering incidencedegree"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy --ordering incidencedegree --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy --ordering smallestlast"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy --ordering smallestlast --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy --ordering dynamiclargestfirst"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy --ordering dynamiclargestfirst --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy_dsatur"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "localsearch_rowweighting --iterations-without-improvment 1000"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "localsearch_rowweighting --iterations-without-improvment 10000"
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristicshort --timelimit 2 --labels "greedy --ordering largestfirst" "greedy --ordering largestfirst --reverse" "greedy --ordering incidencedegree" "greedy --ordering incidencedegree --reverse" "greedy --ordering smallestlast" "greedy --ordering smallestlast --reverse" "greedy --ordering dynamiclargestfirst" "greedy --ordering dynamiclargestfirst --reverse"
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristicshort --timelimit 6 --labels "greedy --ordering largestfirst" "greedy --ordering incidencedegree --reverse" "greedy --ordering smallestlast --reverse" "greedy --ordering dynamiclargestfirst" "greedy_dsatur" "localsearch_rowweighting --iterations-without-improvement 1000" "localsearch_rowweighting --iterations-without-improvement 10000"
```

![heuristicshort1](img/heuristicshort1.png?raw=true "heuristicshort1")

![heuristicshort2](img/heuristicshort2.png?raw=true "heuristicshort2")

```shell
python3 ../optimizationtools/optimizationtools/bench_run.py --timelimit 3600 --algorithms "milp_assignment_cplex"
python3 ../optimizationtools/optimizationtools/bench_run.py --timelimit 3600 --algorithms "milp_representatives_cplex"
python3 ../optimizationtools/optimizationtools/bench_run.py --timelimit 3600 --algorithms "milp_partialordering_cplex"
python3 ../optimizationtools/optimizationtools/bench_run.py --timelimit 3600 --algorithms "milp_partialordering2_cplex"
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark exact --timelimit 3600 --labels "milp_assignment_cplex" "milp_representatives_cplex" "milp_partialordering_cplex" "milp_partialordering2_cplex"
```

```shell
python3 ../optimizationtools/optimizationtools/bench_run.py --timelimit 3600 --algorithms "localsearch_rowweighting"
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristiclong --timelimit 3600 --labels "milp_assignment_cplex" "milp_representatives_cplex" "milp_partialordering_cplex" "milp_partialordering2_cplex" "localsearch_rowweighting"
```

