#include "coloringsolver/algorithms/greedy.hpp"

#include "optimizationtools/indexed_binary_heap.hpp"

using namespace coloringsolver;

std::istream& coloringsolver::operator>>(std::istream& in, Ordering& ordering)
{
    std::string token;
    in >> token;
    if (token == "default") {
        ordering = Ordering::Default;
    } else if (token == "largestfirst" || token == "lf") {
        ordering = Ordering::LargestFirst;
    } else if (token == "incidencedegree" || token == "id") {
        ordering = Ordering::IncidenceDegree;
    } else if (token == "smallestlast" || token == "sl") {
        ordering = Ordering::SmallestLast;
    } else if (token == "dynamiclargestfirst" || token == "dlf") {
        ordering = Ordering::DynamicLargestFirst;
    } else  {
        in.setstate(std::ios_base::failbit);
    }
    return in;
}

std::ostream& coloringsolver::operator<<(std::ostream &os, Ordering ordering)
{
    switch (ordering) {
    case Ordering::Default: {
        os << "default";
        break;
    } case Ordering::LargestFirst: {
        os << "largestfirst";
        break;
    } case Ordering::IncidenceDegree: {
        os << "incidencedegree";
        break;
    } case Ordering::SmallestLast: {
        os << "smallestlast";
        break;
    } case Ordering::DynamicLargestFirst: {
        os << "dynamiclargestfirst";
        break;
    }
    }
    return os;
}

std::vector<VertexId> largestfirst(const Instance& instance)
{
    std::vector<VertexId> ordered_vertices(instance.vertex_number());
    std::vector<std::vector<VertexId>> vertices(instance.degree_max() + 1);
    for (VertexId v = 0; v < instance.vertex_number(); ++v)
        vertices[instance.degree(v)].push_back(v);

    VertexId d_cur = instance.degree_max();
    for (VertexPos v_pos = 0; v_pos < instance.vertex_number(); ++v_pos) {
        while (vertices[d_cur].empty())
            d_cur--;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        ordered_vertices[v_pos] = v;
    }
    return ordered_vertices;
}

std::vector<VertexId> incidencedegree(const Instance& instance)
{
    std::vector<VertexId> ordered_vertices(instance.vertex_number());
    std::vector<uint8_t> added(instance.vertex_number(), 0);
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
    for (VertexPos v_pos = 0; v_pos < instance.vertex_number(); ++v_pos) {
        while (vertices[d_cur].empty())
            d_cur++;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[v] = {-1, -1};
        for (const auto& edge: instance.vertex(v).edges) {
            if (added[edge.v] == 1)
                continue;
            auto old = positions[edge.v];
            positions[vertices[old.first].back()] = old;
            vertices[old.first][old.second] = vertices[old.first].back();
            vertices[old.first].pop_back();
            positions[edge.v] = {old.first + 1, vertices[old.first + 1].size()};
            vertices[old.first + 1].push_back(edge.v);
        }
        added[v] = 1;
        ordered_vertices[v_pos] = v;
    }
    return ordered_vertices;
}

std::vector<VertexId> smallestlast(const Instance& instance)
{
    std::vector<VertexId> ordered_vertices(instance.vertex_number());
    std::vector<uint8_t> added(instance.vertex_number(), 0);

    std::vector<std::vector<VertexId>> vertices(instance.degree_max() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(instance.vertex_number(), {-1, -1});
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        VertexId dv = instance.degree(v);
        positions[v] = {dv, vertices[dv].size()};
        vertices[dv].push_back(v);
    }

    VertexId d_cur = 0;
    for (VertexPos v_pos = 0; v_pos < instance.vertex_number(); ++v_pos) {
        if (d_cur > 0 && !vertices[d_cur - 1].empty())
            d_cur--;
        while (vertices[d_cur].empty())
            d_cur++;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[v] = {-1, -1};
        for (const auto& edge: instance.vertex(v).edges) {
            if (added[edge.v] == 1)
                continue;
            auto old = positions[edge.v];
            positions[vertices[old.first].back()] = old;
            vertices[old.first][old.second] = vertices[old.first].back();
            vertices[old.first].pop_back();
            positions[edge.v] = {old.first - 1, vertices[old.first - 1].size()};
            vertices[old.first - 1].push_back(edge.v);
        }
        added[v] = 1;
        ordered_vertices[v_pos] = v;
    }
    return ordered_vertices;
}

std::vector<VertexId> dynamiclargestfirst(const Instance& instance)
{
    std::vector<VertexId> ordered_vertices(instance.vertex_number());
    std::vector<uint8_t> added(instance.vertex_number(), 0);
    std::vector<std::vector<VertexId>> vertices(instance.degree_max() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(instance.vertex_number(), {-1, -1});
    for (VertexId v = 0; v < instance.vertex_number(); ++v) {
        VertexId dv = instance.degree(v);
        positions[v] = {dv, vertices[dv].size()};
        vertices[dv].push_back(v);
    }

    VertexId d_cur = instance.degree_max();
    for (VertexPos v_pos = 0; v_pos < instance.vertex_number(); ++v_pos) {
        while (vertices[d_cur].empty())
            d_cur--;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[v] = {-1, -1};
        for (const auto& edge: instance.vertex(v).edges) {
            if (added[edge.v] == 1)
                continue;
            auto old = positions[edge.v];
            positions[vertices[old.first].back()] = old;
            vertices[old.first][old.second] = vertices[old.first].back();
            vertices[old.first].pop_back();
            positions[edge.v] = {old.first - 1, vertices[old.first - 1].size()};
            vertices[old.first - 1].push_back(edge.v);
        }
        added[v] = 1;
        ordered_vertices[v_pos] = v;
    }
    return ordered_vertices;
}

Output coloringsolver::greedy(const Instance& instance, GreedyOptionalParameters parameters)
{
    VER(parameters.info, "*** greedy --ordering " << parameters.ordering
            << ((parameters.reverse)? " --reverse": "")
            << " ***" << std::endl);
    Output output(instance, parameters.info);
    Solution solution(instance);

    std::vector<VertexId> ordered_vertices;
    switch (parameters.ordering) {
    case Ordering::Default: {
        ordered_vertices.resize(instance.vertex_number());
        std::iota(ordered_vertices.begin(), ordered_vertices.end(), 0);
        break;
    } case Ordering::LargestFirst: {
        ordered_vertices = largestfirst(instance);
        break;
    } case Ordering::IncidenceDegree: {
        ordered_vertices = incidencedegree(instance);
        break;
    } case Ordering::SmallestLast: {
        ordered_vertices = smallestlast(instance);
        break;
    } case Ordering::DynamicLargestFirst: {
        ordered_vertices = dynamiclargestfirst(instance);
        break;
    } default: {
    }
    }

    optimizationtools::IndexedSet color_set(instance.degree_max());
    if (!parameters.reverse) {
        for (auto it_v = ordered_vertices.begin(); it_v != ordered_vertices.end(); ++it_v) {
            VertexId v = *it_v;
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
    } else {
        for (auto it_v = ordered_vertices.rbegin(); it_v != ordered_vertices.rend(); ++it_v) {
            VertexId v = *it_v;
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
    }

    output.update_solution(solution, std::stringstream(), parameters.info);
    return output.algorithm_end(parameters.info);
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

