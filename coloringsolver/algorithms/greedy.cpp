#include "coloringsolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_binary_heap.hpp"

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
    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();

    std::vector<VertexId> ordered_vertices(n);
    std::vector<std::vector<VertexId>> vertices(graph.maximum_degree() + 1);
    for (VertexId v = 0; v < n; ++v)
        vertices[graph.degree(v)].push_back(v);

    VertexId d_cur = graph.maximum_degree();
    for (VertexPos v_pos = 0; v_pos < n; ++v_pos) {
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
    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();

    std::vector<VertexId> ordered_vertices(n);
    std::vector<uint8_t> added(n, 0);
    std::vector<std::vector<VertexId>> vertices(graph.maximum_degree() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(n, {-1, -1});
    VertexId v_best = -1;
    for (VertexId v = 0; v < n; ++v) {
        positions[v] = {0, v};
        vertices[0].push_back(v);
        if (v_best == -1 || graph.degree(v_best) < graph.degree(v))
            v_best = v;
    }
    vertices[0][n - 1] = v_best;
    vertices[0][v_best] = n - 1;
    positions[v_best].second = n - 1;
    positions[n - 1].second = v_best;

    VertexId d_cur = 0;
    for (VertexPos v_pos = 0; v_pos < n; ++v_pos) {
        while (vertices[d_cur].empty())
            d_cur++;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[v] = {-1, -1};
        auto it = graph.neighbors_begin(v);
        auto it_end = graph.neighbors_end(v);
        for (; it != it_end; ++it) {
            VertexId v_neighbor = *it;
            if (added[v_neighbor] == 1)
                continue;
            auto old = positions[v_neighbor];
            positions[vertices[old.first].back()] = old;
            vertices[old.first][old.second] = vertices[old.first].back();
            vertices[old.first].pop_back();
            positions[v_neighbor] = {old.first + 1, vertices[old.first + 1].size()};
            vertices[old.first + 1].push_back(v_neighbor);
        }
        added[v] = 1;
        ordered_vertices[v_pos] = v;
    }
    return ordered_vertices;
}

std::vector<VertexId> smallestlast(const Instance& instance)
{
    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();

    std::vector<VertexId> ordered_vertices(n);
    std::vector<uint8_t> added(n, 0);

    std::vector<std::vector<VertexId>> vertices(graph.maximum_degree() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(n, {-1, -1});
    for (VertexId v = 0; v < n; ++v) {
        VertexId dv = graph.degree(v);
        positions[v] = {dv, vertices[dv].size()};
        vertices[dv].push_back(v);
    }

    VertexId d_cur = 0;
    for (VertexPos v_pos = 0; v_pos < n; ++v_pos) {
        if (d_cur > 0 && !vertices[d_cur - 1].empty())
            d_cur--;
        while (vertices[d_cur].empty())
            d_cur++;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[v] = {-1, -1};
        auto it = graph.neighbors_begin(v);
        auto it_end = graph.neighbors_end(v);
        for (; it != it_end; ++it) {
            VertexId v_neighbor = *it;
            if (added[v_neighbor] == 1)
                continue;
            auto old = positions[v_neighbor];
            positions[vertices[old.first].back()] = old;
            vertices[old.first][old.second] = vertices[old.first].back();
            vertices[old.first].pop_back();
            positions[v_neighbor] = {old.first - 1, vertices[old.first - 1].size()};
            vertices[old.first - 1].push_back(v_neighbor);
        }
        added[v] = 1;
        ordered_vertices[v_pos] = v;
    }
    return ordered_vertices;
}

std::vector<VertexId> dynamiclargestfirst(const Instance& instance)
{
    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();

    std::vector<VertexId> ordered_vertices(n);
    std::vector<uint8_t> added(n, 0);
    std::vector<std::vector<VertexId>> vertices(graph.maximum_degree() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(n, {-1, -1});
    for (VertexId v = 0; v < n; ++v) {
        VertexId dv = graph.degree(v);
        positions[v] = {dv, vertices[dv].size()};
        vertices[dv].push_back(v);
    }

    VertexId d_cur = graph.maximum_degree();
    for (VertexPos v_pos = 0; v_pos < n; ++v_pos) {
        while (vertices[d_cur].empty())
            d_cur--;
        VertexId v = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[v] = {-1, -1};
        auto it = graph.neighbors_begin(v);
        auto it_end = graph.neighbors_end(v);
        for (; it != it_end; ++it) {
            VertexId v_neighbor = *it;
            if (added[v_neighbor] == 1)
                continue;
            auto old = positions[v_neighbor];
            positions[vertices[old.first].back()] = old;
            vertices[old.first][old.second] = vertices[old.first].back();
            vertices[old.first].pop_back();
            positions[v_neighbor] = {old.first - 1, vertices[old.first - 1].size()};
            vertices[old.first - 1].push_back(v_neighbor);
        }
        added[v] = 1;
        ordered_vertices[v_pos] = v;
    }
    return ordered_vertices;
}

Output coloringsolver::greedy(const Instance& instance, GreedyOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    FFOT_VER(parameters.info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "Greedy" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Ordering:      " << parameters.ordering << std::endl
            << "Reverse:       " << parameters.reverse << std::endl
            << std::endl);

    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();
    Output output(instance, parameters.info);
    Solution solution(instance);

    std::vector<VertexId> ordered_vertices;
    switch (parameters.ordering) {
    case Ordering::Default: {
        ordered_vertices.resize(n);
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

    optimizationtools::IndexedSet color_set(graph.maximum_degree() + 1);
    if (!parameters.reverse) {
        for (auto it_v = ordered_vertices.begin(); it_v != ordered_vertices.end(); ++it_v) {
            VertexId v = *it_v;
            color_set.clear();
            auto it = graph.neighbors_begin(v);
            auto it_end = graph.neighbors_end(v);
            for (; it != it_end; ++it) {
                VertexId v_neighbor = *it;
                if (solution.contains(v_neighbor))
                    color_set.add(solution.color(v_neighbor));
            }
            ColorId c_best = -1;
            for (ColorId c = 0; c < n; ++c) {
                if (!color_set.contains(c)) {
                    c_best = c;
                    break;
                }
            }
            solution.set(v, c_best, false);
        }
    } else {
        for (auto it_v = ordered_vertices.rbegin(); it_v != ordered_vertices.rend(); ++it_v) {
            VertexId v = *it_v;
            color_set.clear();
            auto it = graph.neighbors_begin(v);
            auto it_end = graph.neighbors_end(v);
            for (; it != it_end; ++it) {
                VertexId v_neighbor = *it;
                if (solution.contains(v_neighbor))
                    color_set.add(solution.color(v_neighbor));
            }
            ColorId c_best = -1;
            for (ColorId c = 0; c < n; ++c) {
                if (!color_set.contains(c)) {
                    c_best = c;
                    break;
                }
            }
            solution.set(v, c_best, false);
        }
    }

    output.update_solution(solution, std::stringstream(), parameters.info);
    return output.algorithm_end(parameters.info);
}

Output coloringsolver::greedy_dsatur(
        const Instance& instance,
        optimizationtools::Info info)
{
    init_display(instance, info);
    FFOT_VER(info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "DSATUR" << std::endl
            << std::endl);

    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();
    Output output(instance, info);
    Solution solution(instance);

    VertexId v_best = -1;
    for (VertexId v = 0; v < n; ++v)
        if (v_best == -1 || graph.degree(v_best) < graph.degree(v))
            v_best = v;

    auto f = [](VertexId) { return 0; };
    optimizationtools::IndexedBinaryHeap<double> heap(n, f);
    heap.update_key(v_best, -1);

    optimizationtools::IndexedSet color_set(graph.maximum_degree() + 1);

    std::vector<std::vector<bool>> is_adjacent;
    std::vector<ColorId> number_of_adjacent_colors(n, 0);
    while (!solution.feasible()) {
        auto p = heap.top();
        heap.pop();

        color_set.clear();
        VertexId v = p.first;
        auto it_begin = graph.neighbors_begin(v);
        auto it_end = graph.neighbors_end(v);
        for (auto it = it_begin; it != it_end; ++it) {
            VertexId v_neighbor = *it;
            if (solution.contains(v_neighbor))
                color_set.add(solution.color(v_neighbor));
        }

        ColorId c_best = -1;
        for (ColorId c = 0; c < n; ++c) {
            if (!color_set.contains(c)) {
                c_best = c;
                break;
            }
        }
        if (c_best >= (ColorId)is_adjacent.size()) {
            is_adjacent.push_back(std::vector<bool>(n, false));
        }

        solution.set(p.first, c_best, false);

        for (auto it = it_begin; it != it_end; ++it) {
            VertexId v_neighbor = *it;
            if (solution.contains(v_neighbor))
                continue;
            if (is_adjacent[c_best][v_neighbor])
                continue;
            is_adjacent[c_best][v_neighbor] = true;
            number_of_adjacent_colors[v_neighbor]++;
            double val = -number_of_adjacent_colors[v_neighbor]
                - (double)graph.degree(v_neighbor) / (graph.maximum_degree() + 1);
            heap.update_key(v_neighbor, val);
        }
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

