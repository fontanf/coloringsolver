# ColoringSolver

A solver for the Graph Coloring Problem

## Implemented algorithms

* Greedy Largest First `-a greedy_largestfirst`
* Greedy Smallest Last `-a greedy_smallestlast`
* Greedy DSATUR `-a greedy_dsatur`
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

