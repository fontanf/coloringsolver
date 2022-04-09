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
  * Penalize conflicting edges `-a "localsearch_rowweighting --iterations 100000 --iterations-without-improvement 10000"`
  * Penalize uncolored vertices `-a "localsearch_rowweighting_2 --iterations 100000 --iterations-without-improvement 10000"`

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

Examples:

```shell
./bazel-bin/coloringsolver/main -v -i "data/dimacs1992/1-FullIns_3.col" -a greedy_dsatur -c solution.txt
```
```
=====================================
           Coloring Solver           
=====================================

Instance
--------
Name:                data/graphcoloring/1-FullIns_3.col
Number of vertices:  30
Number of edges:     100
Density:             0.229885
Average degree:      6.66667
Maximum degree:      11

Algorithm
---------
DSATUR

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
      0.0001          12           0          12         inf                        
      0.0001           4           0           4         inf                        

Final statistics
----------------
Value:                 4
Bound:                 0
Gap:                   4
Gap (%):               inf
Time (s):              0.0002
```

```shell
./bazel-bin/coloringsolver/main -v -i "data/dimacs1992/r1000.5.col" -a "localsearch_rowweighting" -t 30
```
```
=====================================
           Coloring Solver           
=====================================

Instance
--------
Name:                data/graphcoloring/r1000.5.col
Number of vertices:  1000
Number of edges:     238267
Density:             0.477011
Average degree:      476.534
Maximum degree:      781

Algorithm
---------
Row Weighting Local Search

Parameters
----------
Maximum number of iterations:                      -1
Maximum number of iterations without improvement:  -1
Maximum number of improvements:                    -1
Goal:                                              0

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
      0.0001         782           0         782         inf                        
      0.0097         250           0         250         inf        initial solution
      0.0113         249           0         249         inf             iteration 7
      0.0126         248           0         248         inf            iteration 11
      0.0139         247           0         247         inf            iteration 41
      0.0155         246           0         246         inf           iteration 115
      0.8448         245           0         245         inf        iteration 182052
      0.9065         244           0         244         inf        iteration 196035
      1.4955         243           0         243         inf        iteration 335080
      1.5796         242           0         242         inf        iteration 354950
      1.8055         241           0         241         inf        iteration 408907
      2.3594         240           0         240         inf        iteration 543664
      3.0347         239           0         239         inf        iteration 712696
      3.4089         238           0         238         inf        iteration 806580
      4.2307         237           0         237         inf       iteration 1016132
      7.3668         236           0         236         inf       iteration 1832197
     10.2384         235           0         235         inf       iteration 2596244
     17.5431         234           0         234         inf       iteration 4565602

Final statistics
----------------
Value:                 234
Bound:                 0
Gap:                   234
Gap (%):               inf
Time (s):              20
Number of iterations:  5212819
```

Benchmarks:
```shell
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering largestfirst"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering largestfirst --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering incidencedegree"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering incidencedegree --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering smallestlast"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering smallestlast --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering dynamiclargestfirst"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy --ordering dynamiclargestfirst --reverse"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "greedy_dsatur"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "localsearch_rowweighting --iterations-without-improvement 1000"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "localsearch_rowweighting --iterations-without-improvement 10000"
python3 ../optimizationtools/optimizationtools/bench_run.py -f "row['Dataset'] == 'dimacs1992'" --algorithms "localsearch_rowweighting --iterations-without-improvement 100000"
python3 ../optimizationtools/optimizationtools/bench_process.py -f "row['Dataset'] == 'dimacs1992'" --benchmark heuristicshort --timelimit 0.25 --labels "greedy --ordering largestfirst" "greedy --ordering largestfirst --reverse" "greedy --ordering incidencedegree" "greedy --ordering incidencedegree --reverse" "greedy --ordering smallestlast" "greedy --ordering smallestlast --reverse" "greedy --ordering dynamiclargestfirst" "greedy --ordering dynamiclargestfirst --reverse" -o heuristicshort1
python3 ../optimizationtools/optimizationtools/bench_process.py -f "row['Dataset'] == 'dimacs1992'" --benchmark heuristicshort --timelimit 2 --labels "greedy --ordering largestfirst" "greedy --ordering incidencedegree --reverse" "greedy --ordering smallestlast --reverse" "greedy --ordering dynamiclargestfirst" "greedy_dsatur" "localsearch_rowweighting --iterations-without-improvement 1000" "localsearch_rowweighting --iterations-without-improvement 10000" -o heuristicshort2
python3 ../optimizationtools/optimizationtools/bench_process.py -f "row['Dataset'] == 'dimacs1992'" --benchmark heuristicshort --timelimit 6 --labels "greedy_dsatur" "localsearch_rowweighting --iterations-without-improvement 1000" "localsearch_rowweighting --iterations-without-improvement 10000" "localsearch_rowweighting --iterations-without-improvement 100000" -o heuristicshort3
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

