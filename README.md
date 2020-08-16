# ColoringSolver

A solver for the Graph Coloring Problem.

## Implemented algorithms

* Greedy algorithms, see "ColPack: Software for graph coloring and related problems in scientific computing" (Gebremedhin et al., 2013) for their descriptions:
  * Largest first `-a greedy_largestfirst`
  * Incidence degree `-a greedy_incidencedegree`
  * Smallest last `-a greedy_smallestlast`
  * Dynamic Largest First `-a greedy_dynamiclargestfirst`
  * DSATUR `-a greedy_dsatur`
* Row weighting local search `-a "localsearch --threads 3"`
* Branch-and-cut (CPLEX) `-a branchandcut_cplex`

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
./bazel-bin/coloringsolver/main -v -i "data/graphcoloring/1-FullIns_3.col" -a branchandcut_cplex -t 60 -c solution.txt
```

