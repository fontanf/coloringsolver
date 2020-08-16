#include "coloringsolver/algorithms/greedy.hpp"

#include "optimizationtools/indexed_binary_heap.hpp"

#include <queue>

using namespace coloringsolver;

Output coloringsolver::greedy_largestfirst(const Instance& instance, Info info)
{
    VER(info, "*** greedy_largestfirst ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    std::vector<std::vector<VertexId>> vertices(instance.degree_max() + 1);
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        vertices[instance.degree(v)].push_back(v);

    VertexId d_cur = instance.degree_max();
    optimizationtools::IndexedSet color_set(instance.degree_max());
    while (!solution.feasible()) {
        while (vertices[d_cur].empty())
            d_cur--;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        color_set.clear();
        for (const auto& edge: instance.vertex(v).edges)
            if (solution.contains(edge.v))
                color_set.add(solution.color(edge.v));
        ColorId c_best = -1;
        for (ColorId c = 0; c < instance.vertex_number(); ++c) {
            if (!color_set.contains(c)) {
                c_best = c;
                break;
            }
        }
        solution.set(v, c_best);
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output coloringsolver::greedy_incidencedegree(const Instance& instance, Info info)
{
    VER(info, "*** greedy_incidencedegree ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    std::vector<std::vector<VertexId>> vertices(instance.degree_max() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(instance.vertex_number(), {-1, -1});
    VertexId v_best = -1;
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        positions[v] = {0, v};
        vertices[0].push_back(v);
        if (v_best == -1 || instance.degree(v_best) < instance.degree(v))
            v_best = v;
    }
    vertices[0][instance.vertex_number() - 1] = v_best;
    vertices[0][v_best] = instance.vertex_number() - 1;
    positions[v_best].second = instance.vertex_number() - 1;
    positions[instance.vertex_number() - 1].second = v_best;

    VertexId d_cur = 0;
    optimizationtools::IndexedSet color_set(instance.degree_max());
    while (!solution.feasible()) {
        while (vertices[d_cur].empty())
            d_cur++;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[v] = {-1, -1};
        color_set.clear();
        for (const auto& edge: instance.vertex(v).edges) {
            if (solution.contains(edge.v)) {
                color_set.add(solution.color(edge.v));
            } else {
                auto old = positions[edge.v];
                positions[vertices[old.first].back()] = old;
                vertices[old.first][old.second] = vertices[old.first].back();
                vertices[old.first].pop_back();
                positions[edge.v] = {old.first + 1, vertices[old.first + 1].size()};
                vertices[old.first + 1].push_back(edge.v);
            }
        }
        ColorId c_best = -1;
        for (ColorId c = 0; c < instance.vertex_number(); ++c) {
            if (!color_set.contains(c)) {
                c_best = c;
                break;
            }
        }
        solution.set(v, c_best);
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output coloringsolver::greedy_smallestlast(const Instance& instance, Info info)
{
    VER(info, "*** greedy_smallestlast ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    std::vector<std::vector<VertexId>> vertices(instance.degree_max() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(instance.vertex_number(), {-1, -1});
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        VertexId dv = instance.degree(v);
        positions[v] = {dv, vertices[dv].size()};
        vertices[dv].push_back(v);
    }

    VertexId d_cur = 0;
    optimizationtools::IndexedSet color_set(instance.degree_max());
    while (!solution.feasible()) {
        while (vertices[d_cur].empty())
            d_cur++;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[v] = {-1, -1};
        color_set.clear();
        for (const auto& edge: instance.vertex(v).edges) {
            if (solution.contains(edge.v)) {
                color_set.add(solution.color(edge.v));
            } else {
                auto old = positions[edge.v];
                positions[vertices[old.first].back()] = old;
                vertices[old.first][old.second] = vertices[old.first].back();
                vertices[old.first].pop_back();
                positions[edge.v] = {old.first - 1, vertices[old.first - 1].size()};
                vertices[old.first - 1].push_back(edge.v);
                if (d_cur > old.first - 1)
                    d_cur--;
            }
        }
        ColorId c_best = -1;
        for (ColorId c = 0; c < instance.vertex_number(); ++c) {
            if (!color_set.contains(c)) {
                c_best = c;
                break;
            }
        }
        solution.set(v, c_best);
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output coloringsolver::greedy_dynamiclargestfirst(const Instance& instance, Info info)
{
    VER(info, "*** greedy_dynamiclargestfirst ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    std::vector<std::vector<VertexId>> vertices(instance.degree_max() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(instance.vertex_number(), {-1, -1});
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        VertexId dv = instance.degree(v);
        positions[v] = {dv, vertices[dv].size()};
        vertices[dv].push_back(v);
    }

    VertexId d_cur = instance.degree_max();
    optimizationtools::IndexedSet color_set(instance.degree_max());
    while (!solution.feasible()) {
        while (vertices[d_cur].empty())
            d_cur--;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[v] = {-1, -1};
        color_set.clear();
        for (const auto& edge: instance.vertex(v).edges) {
            if (solution.contains(edge.v)) {
                color_set.add(solution.color(edge.v));
            } else {
                auto old = positions[edge.v];
                positions[vertices[old.first].back()] = old;
                vertices[old.first][old.second] = vertices[old.first].back();
                vertices[old.first].pop_back();
                positions[edge.v] = {old.first - 1, vertices[old.first - 1].size()};
                vertices[old.first - 1].push_back(edge.v);
            }
        }
        ColorId c_best = -1;
        for (ColorId c = 0; c < instance.vertex_number(); ++c) {
            if (!color_set.contains(c)) {
                c_best = c;
                break;
            }
        }
        solution.set(v, c_best);
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

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

    optimizationtools::IndexedSet color_set(instance.degree_max());

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

