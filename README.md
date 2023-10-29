# ColoringSolver

A solver for the graph coloring problem.

![graphcoloring](img/graphcoloring.png?raw=true "graphcoloring")

[image source](https://commons.wikimedia.org/wiki/File:Petersen_graph_3-coloring.svg)

## Implemented algorithms

* Greedy algorithms, see "ColPack: Software for graph coloring and related problems in scientific computing" (Gebremedhin et al., 2013) for their descriptions:
  * Largest first `-a greedy --ordering largest-first`
  * Incidence degree `-a greedy --ordering incidence-degree --reverse`
  * Smallest last `-a greedy --ordering smallest-last --reverse`
  * Dynamic largest first `-a greedy --ordering dynamic-largest-first`
  * DSATUR `-a greedy-dsatur`

* MILP (CPLEX), see "New Integer Linear Programming Models for the Vertex Coloring Problem" (Jabrayilov et Mutzel, 2018) for model descriptions:
  * Assignment-based ILP model `-a milp-assignment-cplex`
  * Representatives ILP model `-a milp-representatives-cplex`
  * Partial-ordering based ILP model `-a milp-partial-ordering-cplex`
  * Partial-ordering based ILP model 2 `-a milp-partial-ordering-2-cplex`

* Row weighting local search
  * Penalize conflicting edges `-a "local-search-row-weighting --iterations 100000 --iterations-without-improvement 10000"`
  * Penalize uncolored vertices `-a "local-search-row-weighting-2 --iterations 100000 --iterations-without-improvement 10000"`

* Column generation heuristics implemented with [fontanf/columngenerationsolver](https://github.com/fontanf/columngenerationsolver):
  * Greedy `column-generation-heuristic-greedy`
  * Limited discrepancy search `column-generation-heuristic-limited-discrepancy-search`

## Usage (command line)

Download and uncompress the instances in the `data/` folder:

https://drive.google.com/file/d/1ZT2dSIleWN__MZIscsc3kAqGRVU91CDw/view?usp=sharing

Compile:
```shell
bazel build -- //...
```

Examples:

```shell
./bazel-bin/coloringsolver/main -v 1 -i "data/dimacs1992/1-FullIns_3.col" -a greedy-dsatur -c solution.txt
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
./bazel-bin/coloringsolver/main -v 1 -i "data/dimacs1992/r1000.5.col" -a "local-search-row-weighting-2 --iterations 50000"
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

Benchmarks:
```shell
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering largest-first"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering largest-first --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering incidence-degree"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering incidence-degree --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering smallest-last"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering smallest-last --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering dynamic-largest-first"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering dynamic-largest-first --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy-dsatur"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "local-search-row-weighting --iterations-without-improvement 1000"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "local-search-row-weighting --iterations-without-improvement 10000"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "local-search-row-weighting --iterations-without-improvement 100000"
python3 ../optimizationtools/optimizationtools/bench_process.py -f "row['Dataset'] == 'dimacs1992'" --benchmark heuristicshort --timelimit 0.25 --labels "greedy --ordering largest-first" "greedy --ordering largest-first --reverse" "greedy --ordering incidence-degree" "greedy --ordering incidence-degree --reverse" "greedy --ordering smallest-last" "greedy --ordering smallest-last --reverse" "greedy --ordering dynamic-largest-first" "greedy --ordering dynamic-largest-first --reverse" -o heuristicshort1
python3 ../optimizationtools/optimizationtools/bench_process.py -f "row['Dataset'] == 'dimacs1992'" --benchmark heuristicshort --timelimit 2 --labels "greedy --ordering largest-first" "greedy --ordering incidence-degree --reverse" "greedy --ordering smallest-last --reverse" "greedy --ordering dynamic-largest-first" "greedy-dsatur" "local-search-row-weighting --iterations-without-improvement 1000" "local-search-row-weighting --iterations-without-improvement 10000" -o heuristicshort2
python3 ../optimizationtools/optimizationtools/bench_process.py -f "row['Dataset'] == 'dimacs1992'" --benchmark heuristicshort --timelimit 6 --labels "greedy-dsatur" "local-search-row-weighting --iterations-without-improvement 1000" "local-search-row-weighting --iterations-without-improvement 10000" "local-search-row-weighting --iterations-without-improvement 100000" -o heuristicshort3
```

![heuristicshort1](img/heuristicshort1.png?raw=true "heuristicshort1")

![heuristicshort2](img/heuristicshort2.png?raw=true "heuristicshort2")

![heuristicshort3](img/heuristicshort3.png?raw=true "heuristicshort3")

```shell
python3 ../optimizationtools/optimizationtools/bench_run.py -l "localsearch_rowweighting_10000" -a "localsearch_rowweighting -w 10000"
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristicshort -l "localsearch_rowweighting_10000" -f "row['Dataset'] == 'graphcoloring'" -t 3
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristicshort -l "localsearch_rowweighting_10000" -f "row['Dataset'] == 'gebremedhin2013'" -t 10
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristicshort -l "localsearch_rowweighting_10000" -f "row['Dataset'] == 'rossi2014'" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristicshort -l "localsearch_rowweighting_10000" -f "'verma2015' in row['Dataset']" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristicshort -l "localsearch_rowweighting_10000" -f "'cgshop2022' in row['Dataset']" -t 120
```

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

