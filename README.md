# Coloring Solver

A solver for the Graph Coloring Problem.

![knapsack](graphcoloring.png?raw=true "graphcoloring")

[image source](https://commons.wikimedia.org/wiki/File:Petersen_graph_3-coloring.svg)

## Implemented algorithms

* Greedy algorithms, see "ColPack: Software for graph coloring and related problems in scientific computing" (Gebremedhin et al., 2013) for their descriptions:
  * Largest first `-a greedy_largestfirst`
  * Incidence degree `-a greedy_incidencedegree`
  * Smallest last `-a greedy_smallestlast`
  * Dynamic largest first `-a greedy_dynamiclargestfirst`
  * DSATUR `-a greedy_dsatur`
* Branch-and-cut (CPLEX), see "New Integer Linear Programming Models for the Vertex Coloring Problem" (Jabrayilov et Mutzel, 2018) for model descriptions:
  * Assignment-based ILP model `-a branchandcut_assignment_cplex`
  * Representatives ILP model `-a branchandcut_representatives_cplex`
  * Partial-ordering based ILP model `-a branchandcut_partialordering_cplex`
  * Partial-ordering based ILP model 2 `-a branchandcut_partialordering2_cplex`
* Row weighting local search `-a "localsearch --threads 3 --iteration-limit 100000 --iteration-without-improvment-limit 10000"`

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
./bazel-bin/coloringsolver/main -v -i "data/graphcoloring/r1000.5.col" -a "localsearch --threads 3"
./bazel-bin/coloringsolver/main -v -i "data/graphcoloring/1-FullIns_3.col" -a branchandcut_assignment_cplex -t 60 -c solution.txt
```

Benchmarks:
```shell
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy_largestfirst"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy_incidencedegree"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy_smallestlast"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy_dynamiclargestfirst"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy_dsatur"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "localsearch --iteration-without-improvment-limit 1000"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "localsearch --iteration-without-improvment-limit 10000"
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristicshort --timelimit 1 --labels "greedy_largestfirst" "greedy_incidencedegree" "greedy_smallestlast" "greedy_dynamiclargestfirst" "greedy_dsatur" "localsearch --iteration-without-improvment-limit 1000" "localsearch --iteration-without-improvment-limit 10000"
```

```shell
python3 ../optimizationtools/optimizationtools/bench_run.py --timelimit 3600 --algorithms "branchandcut_assignment_cplex"
python3 ../optimizationtools/optimizationtools/bench_run.py --timelimit 3600 --algorithms "branchandcut_representatives_cplex"
python3 ../optimizationtools/optimizationtools/bench_run.py --timelimit 3600 --algorithms "branchandcut_partialordering_cplex"
python3 ../optimizationtools/optimizationtools/bench_run.py --timelimit 3600 --algorithms "branchandcut_partialordering2_cplex"
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark exact --timelimit 3600 --labels "branchandcut_assignment_cplex" "branchandcut_representatives_cplex" "branchandcut_partialordering_cplex" "branchandcut_partialordering2_cplex"
```

```shell
python3 ../optimizationtools/optimizationtools/bench_run.py --timelimit 3600 --algorithms "localsearch"
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristiclong --timelimit 3600 --labels "branchandcut_assignment_cplex" "branchandcut_representatives_cplex" "branchandcut_partialordering_cplex" "branchandcut_partialordering2_cplex" "localsearch"
```

