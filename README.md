# ColoringSolver

A solver for the Graph Coloring Problem.

![knapsack](graphcoloring.png?raw=true "graphcoloring")

[image source](https://commons.wikimedia.org/wiki/File:Petersen_graph_3-coloring.svg)

## Implemented algorithms

* Greedy algorithms, see "ColPack: Software for graph coloring and related problems in scientific computing" (Gebremedhin et al., 2013) for their descriptions:
  * Largest first `-a greedy_largestfirst`
  * Incidence degree `-a greedy_incidencedegree`
  * Smallest last `-a greedy_smallestlast`
  * Dynamic Largest First `-a greedy_dynamiclargestfirst`
  * DSATUR `-a greedy_dsatur`
* Branch-and-cut (CPLEX), see "New Integer Linear Programming Models for the Vertex Coloring Problem" (Jabrayilov et Mutzel, 2018) for model descriptions:
  * Assignment-based ILP model `-a branchandcut_assignment_cplex`
  * Representatives ILP model `-a branchandcut_representatives_cplex`
  * Partial-ordering based ILP model `-a branchandcut_partialordering_cplex`
  * Partial-ordering based ILP model 2 `-a branchandcut_partialordering2_cplex`
* Row weighting local search `-a "localsearch --threads 3 --iterations 10000"`

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

