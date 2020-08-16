#include "coloringsolver/algorithms/greedy.hpp"

#include "optimizationtools/indexed_binary_heap.hpp"

#include <queue>

using namespace coloringsolver;

Output coloringsolver::greedy_dsatur(const Instance& instance, Info info)
{
    VER(info, "*** greedy_dsatur ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    VertexId v_best = -1;
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        if (v_best == -1 || instance.vertex(v_best).edges.size() < instance.vertex(v).edges.size())
            v_best = v;

    auto f = [](VertexId v) { (void)v; return 0; };
    optimizationtools::IndexedBinaryHeap<double> heap(instance.vertex_number(), f);
    heap.update_key(v_best, -1);

    optimizationtools::IndexedSet color_set(instance.vertex_number());

    while (!solution.feasible()) {
        auto p = heap.top();
        heap.pop();

        color_set.clear();
        for (const auto& vn: instance.vertex(p.first).edges)
            if (solution.contains(vn.v))
                color_set.add(solution.color(vn.v));

        ColorId c_best = -1;
        for (ColorId c = 0; c < instance.vertex_number(); ++c) {
            if (!color_set.contains(c)) {
                c_best = c;
                break;
            }
        }

        solution.set(p.first, c_best);

        for (const auto& vn1: instance.vertex(p.first).edges) {
            if (solution.contains(vn1.v))
                continue;
            color_set.clear();
            for (const auto& vn2: instance.vertex(vn1.v).edges)
                if (solution.contains(vn2.v))
                    color_set.add(solution.color(vn2.v));
            heap.update_key(vn1.v, - color_set.size() - (double)instance.vertex(vn1.v).edges.size() / (instance.degree_max() + 1));
        }
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

