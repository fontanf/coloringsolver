# ColoringSolver

A solver for the graph coloring problem.

![graphcoloring](img/graphcoloring.png?raw=true "graphcoloring")

[image source](https://commons.wikimedia.org/wiki/File:Petersen_graph_3-coloring.svg)

## Implemented algorithms

* Greedy algorithms, see "ColPack: Software for graph coloring and related problems in scientific computing" (Gebremedhin et al., 2013) for their descriptions:
  * Largest first `-a greedy --ordering largest-first`
  * Incidence degree `-a greedy --ordering incidence-degree --reverse 1`
  * Smallest last `-a greedy --ordering smallest-last --reverse 1`
  * Dynamic largest first `-a greedy --ordering dynamic-largest-first`
  * DSATUR `-a greedy-dsatur`

* MILP (CPLEX), see "New Integer Linear Programming Models for the Vertex Coloring Problem" (Jabrayilov et Mutzel, 2018) for model descriptions:
  * Assignment-based ILP model `-a milp-assignment --break-symmetries 1 --solver highs`
  * Representatives ILP model `-a milp-representatives --solver highs`
  * Partial-ordering based ILP model `-a milp-partial-ordering --hybrid 1 --solver highs`

* Row weighting local search
  * Penalize conflicting edges `-a "local-search-row-weighting --iterations 100000 --iterations-without-improvement 10000"`
  * Penalize uncolored vertices `-a "local-search-row-weighting-2 --iterations 100000 --iterations-without-improvement 10000"`

* Column generation heuristics implemented with [fontanf/columngenerationsolver](https://github.com/fontanf/columngenerationsolver):
  * Greedy `column-generation-heuristic-greedy`
  * Limited discrepancy search `column-generation-heuristic-limited-discrepancy-search`

## Usage (command line)

Compile:
```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
cmake --install build --config Release --prefix install
```

Download data:
```shell
python3 scripts/download_data.py
python3 scripts/download_data.py --data verma2015
```

Examples:

```shell
./install/bin/coloringsolver --verbosity-level 1  --input "data/dimacs1992/1-FullIns_3.col"  --algorithm greedy-dsatur  --certificate solution.txt
```
```
====================================
           ColoringSolver           
====================================

Instance
--------
Number of vertices:  30
Number of edges:     100
Density:             0.222222
Average degree:      6.66667
Maximum degree:      11
Total weight:        30

Algorithm
---------
DSATUR

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
       0.000         inf           0         inf         inf                        
       0.000           4           0           4         inf                        

Final statistics
----------------
Value:                        4
Bound:                        0
Absolute optimality gap:      4
Relative optimality gap (%):  inf
Time (s):                     0.000206739

Solution
--------
Number of vertices:   30 / 30 (100%)
Number of conflicts:  0
Feasible:             1
Number of colors:     4
```

```shell
./install/bin/coloringsolver  --verbosity-level 1  --input "data/dimacs1992/r1000.5.col"  --algorithm local-search-row-weighting-2 --maximum-number-of-iterations 50000
```
```
====================================
           ColoringSolver           
====================================

Instance
--------
Number of vertices:  1000
Number of edges:     238267
Density:             0.476534
Average degree:      476.534
Maximum degree:      781
Total weight:        1000

Algorithm
---------
Row weighting local search 2

Parameters
----------
Maximum number of iterations:                      50000
Maximum number of iterations without improvement:  -1
Maximum number of improvements:                    -1
Goal:                                              0

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
       0.000         inf           0         inf         inf                        
       0.007         250           0         250         inf        initial solution
       0.009         249           0         249         inf            iteration 29
       0.011         248           0         248         inf            iteration 34
       0.026         247           0         247         inf          iteration 2473
       0.029         246           0         246         inf          iteration 2524
       0.032         245           0         245         inf          iteration 2865
       0.038         244           0         244         inf          iteration 3514
       0.041         243           0         243         inf          iteration 3699
       0.043         242           0         242         inf          iteration 3842
       0.047         241           0         241         inf          iteration 4197
       0.053         240           0         240         inf          iteration 4939
       0.065         239           0         239         inf          iteration 7201
       0.070         238           0         238         inf          iteration 7791
       0.082         237           0         237         inf         iteration 10040
       0.090         236           0         236         inf         iteration 11477
       0.148         235           0         235         inf         iteration 24907
       0.224         234           0         234         inf         iteration 42727

Final statistics
----------------
Value:                        234
Bound:                        0
Absolute optimality gap:      234
Relative optimality gap (%):  inf
Time (s):                     0.256734
Number of iterations:         50000

Solution
--------
Number of vertices:   1000 / 1000 (100%)
Number of conflicts:  0
Feasible:             1
Number of colors:     234
```
