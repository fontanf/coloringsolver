#include "coloringsolver/algorithms/greedy.hpp"

#include "coloringsolver/algorithm_formatter.hpp"

#include "optimizationtools/containers/indexed_binary_heap.hpp"

using namespace coloringsolver;

std::istream& coloringsolver::operator>>(
        std::istream& in,
        Ordering& ordering)
{
    std::string token;
    in >> token;
    if (token == "default") {
        ordering = Ordering::Default;
    } else if (token == "largest-first" || token == "lf") {
        ordering = Ordering::LargestFirst;
    } else if (token == "incidence-degree" || token == "id") {
        ordering = Ordering::IncidenceDegree;
    } else if (token == "smallest-last" || token == "sl") {
        ordering = Ordering::SmallestLast;
    } else if (token == "dynamic-largest-first" || token == "dlf") {
        ordering = Ordering::DynamicLargestFirst;
    } else  {
        in.setstate(std::ios_base::failbit);
    }
    return in;
}

std::ostream& coloringsolver::operator<<(
        std::ostream &os,
        Ordering ordering)
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
    std::vector<std::vector<VertexId>> vertices(graph.highest_degree() + 1);
    for (VertexId vertex_id = 0; vertex_id < n; ++vertex_id)
        vertices[graph.degree(vertex_id)].push_back(vertex_id);

    VertexId d_cur = graph.highest_degree();
    for (VertexPos vertex_pos = 0; vertex_pos < n; ++vertex_pos) {
        while (vertices[d_cur].empty())
            d_cur--;
        VertexId vertex_id = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        ordered_vertices[vertex_pos] = vertex_id;
    }
    return ordered_vertices;
}

std::vector<VertexId> incidencedegree(const Instance& instance)
{
    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();

    std::vector<VertexId> ordered_vertices(n);
    std::vector<uint8_t> added(n, 0);
    std::vector<std::vector<VertexId>> vertices(graph.highest_degree() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(n, {-1, -1});
    VertexId vertex_id_best = -1;
    for (VertexId vertex_id = 0; vertex_id < n; ++vertex_id) {
        positions[vertex_id] = {0, vertex_id};
        vertices[0].push_back(vertex_id);
        if (vertex_id_best == -1
                || graph.degree(vertex_id_best) < graph.degree(vertex_id))
            vertex_id_best = vertex_id;
    }
    vertices[0][n - 1] = vertex_id_best;
    vertices[0][vertex_id_best] = n - 1;
    positions[vertex_id_best].second = n - 1;
    positions[n - 1].second = vertex_id_best;

    VertexId d_cur = 0;
    for (VertexPos vertex_pos = 0; vertex_pos < n; ++vertex_pos) {
        while (vertices[d_cur].empty())
            d_cur++;
        VertexId vertex_id = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[vertex_id] = {-1, -1};
        auto it = graph.neighbors_begin(vertex_id);
        auto it_end = graph.neighbors_end(vertex_id);
        for (; it != it_end; ++it) {
            VertexId vertex_id_neighbor = *it;
            if (added[vertex_id_neighbor] == 1)
                continue;
            auto old = positions[vertex_id_neighbor];
            positions[vertices[old.first].back()] = old;
            vertices[old.first][old.second] = vertices[old.first].back();
            vertices[old.first].pop_back();
            positions[vertex_id_neighbor] = {old.first + 1, vertices[old.first + 1].size()};
            vertices[old.first + 1].push_back(vertex_id_neighbor);
        }
        added[vertex_id] = 1;
        ordered_vertices[vertex_pos] = vertex_id;
    }
    return ordered_vertices;
}

std::vector<VertexId> smallestlast(const Instance& instance)
{
    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();

    std::vector<VertexId> ordered_vertices(n);
    std::vector<uint8_t> added(n, 0);

    std::vector<std::vector<VertexId>> vertices(graph.highest_degree() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(n, {-1, -1});
    for (VertexId vertex_id = 0; vertex_id < n; ++vertex_id) {
        VertexId dv = graph.degree(vertex_id);
        positions[vertex_id] = {dv, vertices[dv].size()};
        vertices[dv].push_back(vertex_id);
    }

    VertexId d_cur = 0;
    for (VertexPos vertex_pos = 0; vertex_pos < n; ++vertex_pos) {
        if (d_cur > 0 && !vertices[d_cur - 1].empty())
            d_cur--;
        while (vertices[d_cur].empty())
            d_cur++;
        VertexId vertex_id = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[vertex_id] = {-1, -1};
        auto it = graph.neighbors_begin(vertex_id);
        auto it_end = graph.neighbors_end(vertex_id);
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
        added[vertex_id] = 1;
        ordered_vertices[vertex_pos] = vertex_id;
    }
    return ordered_vertices;
}

std::vector<VertexId> dynamiclargestfirst(const Instance& instance)
{
    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();

    std::vector<VertexId> ordered_vertices(n);
    std::vector<uint8_t> added(n, 0);
    std::vector<std::vector<VertexId>> vertices(graph.highest_degree() + 1);
    std::vector<std::pair<VertexId, VertexPos>> positions(n, {-1, -1});
    for (VertexId vertex_id = 0; vertex_id < n; ++vertex_id) {
        VertexId dv = graph.degree(vertex_id);
        positions[vertex_id] = {dv, vertices[dv].size()};
        vertices[dv].push_back(vertex_id);
    }

    VertexId d_cur = graph.highest_degree();
    for (VertexPos vertex_pos = 0; vertex_pos < n; ++vertex_pos) {
        while (vertices[d_cur].empty())
            d_cur--;
        VertexId vertex_id = vertices[d_cur].back();
        vertices[d_cur].pop_back();
        positions[vertex_id] = {-1, -1};
        auto it = graph.neighbors_begin(vertex_id);
        auto it_end = graph.neighbors_end(vertex_id);
        for (; it != it_end; ++it) {
            VertexId vertex_id_neighbor = *it;
            if (added[vertex_id_neighbor] == 1)
                continue;
            auto old = positions[vertex_id_neighbor];
            positions[vertices[old.first].back()] = old;
            vertices[old.first][old.second] = vertices[old.first].back();
            vertices[old.first].pop_back();
            positions[vertex_id_neighbor] = {old.first - 1, vertices[old.first - 1].size()};
            vertices[old.first - 1].push_back(vertex_id_neighbor);
        }
        added[vertex_id] = 1;
        ordered_vertices[vertex_pos] = vertex_id;
    }
    return ordered_vertices;
}

const Output coloringsolver::greedy(
        const Instance& instance,
        const GreedyParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Greedy");
    algorithm_formatter.print_header();

    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();
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

    optimizationtools::IndexedSet color_set(graph.highest_degree() + 1);
    if (!parameters.reverse) {
        for (auto it_v = ordered_vertices.begin(); it_v != ordered_vertices.end(); ++it_v) {
            VertexId vertex_id = *it_v;
            color_set.clear();
            auto it = graph.neighbors_begin(vertex_id);
            auto it_end = graph.neighbors_end(vertex_id);
            for (; it != it_end; ++it) {
                VertexId vertex_id_neighbor = *it;
                if (solution.contains(vertex_id_neighbor))
                    color_set.add(solution.color(vertex_id_neighbor));
            }
            ColorId color_id_best = -1;
            for (ColorId color_id = 0; color_id < n; ++color_id) {
                if (!color_set.contains(color_id)) {
                    color_id_best = color_id;
                    break;
                }
            }
            solution.set(vertex_id, color_id_best, false);
        }
    } else {
        for (auto it_v = ordered_vertices.rbegin(); it_v != ordered_vertices.rend(); ++it_v) {
            VertexId vertex_id = *it_v;
            color_set.clear();
            auto it = graph.neighbors_begin(vertex_id);
            auto it_end = graph.neighbors_end(vertex_id);
            for (; it != it_end; ++it) {
                VertexId vertex_id_neighbor = *it;
                if (solution.contains(vertex_id_neighbor))
                    color_set.add(solution.color(vertex_id_neighbor));
            }
            ColorId color_id_best = -1;
            for (ColorId color_id = 0; color_id < n; ++color_id) {
                if (!color_set.contains(color_id)) {
                    color_id_best = color_id;
                    break;
                }
            }
            solution.set(vertex_id, color_id_best, false);
        }
    }

    algorithm_formatter.update_solution(solution, "");

    algorithm_formatter.end();
    return output;
}

const Output coloringsolver::greedy_dsatur(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("DSATUR");
    algorithm_formatter.print_header();

    const optimizationtools::AbstractGraph& graph = instance.graph();
    VertexId n = graph.number_of_vertices();
    Solution solution(instance);

    VertexId vertex_id_best = -1;
    for (VertexId vertex_id = 0; vertex_id < n; ++vertex_id)
        if (vertex_id_best == -1
                || graph.degree(vertex_id_best) < graph.degree(vertex_id))
            vertex_id_best = vertex_id;

    auto f = [](VertexId) { return 0; };
    optimizationtools::IndexedBinaryHeap<double> heap(n, f);
    heap.update_key(vertex_id_best, -1);

    optimizationtools::IndexedSet color_set(graph.highest_degree() + 1);

    std::vector<std::vector<bool>> is_adjacent;
    std::vector<ColorId> number_of_adjacent_colors(n, 0);
    while (!solution.feasible()) {
        auto p = heap.top();
        heap.pop();

        color_set.clear();
        VertexId vertex_id = p.first;
        auto it_begin = graph.neighbors_begin(vertex_id);
        auto it_end = graph.neighbors_end(vertex_id);
        for (auto it = it_begin; it != it_end; ++it) {
            VertexId vertex_id_neighbor = *it;
            if (solution.contains(vertex_id_neighbor))
                color_set.add(solution.color(vertex_id_neighbor));
        }

        ColorId color_id_best = -1;
        for (ColorId color_id = 0; color_id < n; ++color_id) {
            if (!color_set.contains(color_id)) {
                color_id_best = color_id;
                break;
            }
        }
        if (color_id_best >= (ColorId)is_adjacent.size()) {
            is_adjacent.push_back(std::vector<bool>(n, false));
        }

        solution.set(p.first, color_id_best, false);

        for (auto it = it_begin; it != it_end; ++it) {
            VertexId vertex_id_neighbor = *it;
            if (solution.contains(vertex_id_neighbor))
                continue;
            if (is_adjacent[color_id_best][vertex_id_neighbor])
                continue;
            is_adjacent[color_id_best][vertex_id_neighbor] = true;
            number_of_adjacent_colors[vertex_id_neighbor]++;
            double val = -number_of_adjacent_colors[vertex_id_neighbor]
                - (double)graph.degree(vertex_id_neighbor) / (graph.highest_degree() + 1);
            heap.update_key(vertex_id_neighbor, val);
        }
    }

    algorithm_formatter.update_solution(solution, "");

    algorithm_formatter.end();
    return output;
}
