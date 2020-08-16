# ColoringSolver

A solver for the Graph Coloring Problem

## Implemented algorithms

The greedy algorithms are described in "ColPack: Software for graph coloring and related problems in scientific computing" (Gebremedhin et al., 2013).

* Greedy algorithms:
  * Largest first `-a greedy_largestfirst`
  * Incidence degree `-a greedy_incidencedegree`
  * Smallest last `-a greedy_smallestlast`
  * Dynamic Largest First `-a greedy_dynamiclargestfirst`
  * DSATUR `-a greedy_dsatur`
* Branch-and-cut (CPLEX) `-a branchandcut_cplex`

## Usage (command line)

Compile:
```shell
bazel build -- //...
```

Run:
```shell
./bazel-bin/coloringsolver/main -v -i "data/1-FullIns_3.col" -a greedy_dsatur -c solution.txt
./bazel-bin/coloringsolver/main -v -i "data/1-FullIns_3.col" -a branchandcut_cplex -t 60 -c solution.txt
```

