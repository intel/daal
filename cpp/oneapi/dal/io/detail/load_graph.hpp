/*******************************************************************************
* Copyright 2020-2021 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#pragma once

#include <algorithm>
#include <fstream>

#include <daal/include/services/daal_atomic_int.h>
#include <daal/include/services/daal_memory.h>

#include "oneapi/dal/common.hpp"
#include "oneapi/dal/detail/threading.hpp"
#include "oneapi/dal/exceptions.hpp"
#include "oneapi/dal/graph/common.hpp"
#include "oneapi/dal/graph/detail/undirected_adjacency_vector_graph_impl.hpp"
#include "oneapi/dal/graph/undirected_adjacency_vector_graph.hpp"
#include "oneapi/dal/io/detail/load_graph_service.hpp"
#include "oneapi/dal/io/graph_csv_data_source.hpp"
#include "oneapi/dal/io/load_graph_descriptor.hpp"

namespace oneapi::dal::preview::load_graph::detail {

template <typename Vertex>
inline edge_list<Vertex> load_edge_list(const std::string &name);

template <>
inline edge_list<std::int32_t> load_edge_list(const std::string &name) {
    using int_t = std::int32_t;

    std::ifstream file(name);
    if (!file.is_open()) {
        throw invalid_argument(dal::detail::error_messages::file_not_found());
    }
    edge_list<int_t> elist;
    elist.reserve(1024);

    char source_vertex[32], destination_vertex[32];
    while (file >> source_vertex >> destination_vertex) {
        auto edge = std::make_pair(daal_string_to_int(&source_vertex[0], 0),
                                   daal_string_to_int(&destination_vertex[0], 0));
        elist.push_back(edge);
    }

    file.close();
    return elist;
}

template <typename Index>
std::int64_t get_vertex_count_from_edge_list(const edge_list<Index> &edges) {
    Index max_id = edges[0].first;
    for (std::int64_t i = 0; i < edges.size(); i++) {
        Index edge_max = std::max(edges[i].first, edges[i].second);
        max_id = std::max(max_id, edge_max);
    }

    if (max_id < 0) {
        throw invalid_argument(dal::detail::error_messages::negative_vertex_id());
    }
    const std::int64_t vertex_count = max_id + 1;
    if (vertex_count < 0) {
        throw range_error(dal::detail::error_messages::overflow_found_in_sum_of_two_values());
    }
    return vertex_count;
}

template <typename Index, typename AtomicType>
void collect_degrees_from_edge_list(const edge_list<Index> &edges, AtomicType *&degrees_cv) {
    dal::detail::threader_for(edges.size(), edges.size(), [&](Index u) {
        degrees_cv[edges[u].first].inc();
        degrees_cv[edges[u].second].inc();
    });
}

template <typename EdgeIndex, typename AtomicVertex, typename AtomicEdge>
EdgeIndex compute_prefix_sum_atomic(AtomicVertex *const &degrees,
                                    std::int64_t degrees_count,
                                    AtomicEdge *&edge_offsets_atomic) {
    EdgeIndex total_sum_degrees = 0;
    edge_offsets_atomic[0].set(total_sum_degrees);
    for (std::int64_t i = 0; i < degrees_count; ++i) {
        total_sum_degrees += degrees[i].get();
        edge_offsets_atomic[i + 1].set(total_sum_degrees);
    }
    return total_sum_degrees;
}

template <typename EdgeIndex, typename VertexIndex>
EdgeIndex compute_prefix_sum(VertexIndex *const &degrees,
                             std::int64_t degrees_count,
                             EdgeIndex *&edge_offsets) {
    EdgeIndex total_sum_degrees = 0;
    edge_offsets[0] = total_sum_degrees;
    for (std::int64_t i = 0; i < degrees_count; ++i) {
        total_sum_degrees += degrees[i];
        edge_offsets[i + 1] = total_sum_degrees;
    }
    return total_sum_degrees;
}

template <typename Index, typename AtomicIndex>
void fill_from_atomics(Index *&arr, AtomicIndex *const &atomic_arr, std::int64_t elements_count) {
    dal::detail::threader_for(elements_count, elements_count, [&](Index n) {
        arr[n] = atomic_arr[n].get();
    });
}

template <typename Vertex, typename AtomicEdge>
void fill_unfiltered_neighs(const edge_list<Vertex> &edges,
                            AtomicEdge *&rows_vec_atomic,
                            Vertex *&unfiltered_neighs) {
    dal::detail::threader_for(edges.size(), edges.size(), [&](Vertex u) {
        unfiltered_neighs[rows_vec_atomic[edges[u].first].inc() - 1] = edges[u].second;
        unfiltered_neighs[rows_vec_atomic[edges[u].second].inc() - 1] = edges[u].first;
    });
}

template <typename VertexIndex, typename EdgeIndex>
void fill_filtered_neighs(const EdgeIndex *unfiltered_offsets,
                          const VertexIndex *unfiltered_neighs,
                          const VertexIndex *filtered_degrees,
                          const EdgeIndex *filtered_offsets,
                          VertexIndex *filtered_neighs,
                          std::int64_t vertex_count) {
    dal::detail::threader_for(vertex_count, vertex_count, [&](VertexIndex u) {
        auto u_neighs = filtered_neighs + filtered_offsets[u];
        auto u_neighs_unf = unfiltered_neighs + unfiltered_offsets[u];
        for (VertexIndex i = 0; i < filtered_degrees[u]; i++) {
            u_neighs[i] = u_neighs_unf[i];
        }
    });
}

template <typename VertexIndex, typename EdgeIndex>
void filter_neighbors_and_fill_new_degrees(VertexIndex *unfiltered_neighs,
                                           EdgeIndex *&unfiltered_offsets,
                                           VertexIndex *&new_degrees,
                                           std::int64_t vertex_count) {
    //removing self-loops,  multiple edges from graph, and make neighbors in CSR sorted
    dal::detail::threader_for(vertex_count, vertex_count, [&](VertexIndex u) {
        auto start_p = unfiltered_neighs + unfiltered_offsets[u];
        auto end_p = unfiltered_neighs + unfiltered_offsets[u + 1];

        //dal::detail::parallel_sort(start_p, end_p);
        std::sort(start_p, end_p);
        auto neighs_u_new_end = std::unique(start_p, end_p);
        neighs_u_new_end = std::remove(start_p, neighs_u_new_end, u);
        new_degrees[u] = (VertexIndex)std::distance(start_p, neighs_u_new_end);
    });
}

template <typename Graph>
void convert_to_csr_impl(const edge_list<typename graph_traits<Graph>::vertex_type> &edges,
                         Graph &g) {
    if (edges.size() == 0) {
        throw invalid_argument(dal::detail::error_messages::empty_edge_list());
    }

    using vertex_t = typename graph_traits<Graph>::vertex_type;
    using vertex_size_type = typename graph_traits<Graph>::vertex_size_type;
    using edge_t = typename graph_traits<Graph>::edge_type;
    using atomic_vertex_t = typename daal::services::Atomic<vertex_t>;
    using atomic_edge_t = typename daal::services::Atomic<edge_t>;
    using vertex_allocator_traits =
        typename graph_traits<Graph>::impl_type::vertex_allocator_traits;
    using edge_allocator_traits = typename graph_traits<Graph>::impl_type::edge_allocator_traits;

    using allocator_type = typename graph_traits<Graph>::allocator_type;
    using atomic_vertex_allocator_type =
        typename std::allocator_traits<allocator_type>::template rebind_alloc<atomic_vertex_t>;
    using atomic_vertex_allocator_traits =
        typename std::allocator_traits<allocator_type>::template rebind_traits<atomic_vertex_t>;

    using atomic_edge_allocator_type =
        typename std::allocator_traits<allocator_type>::template rebind_alloc<atomic_edge_t>;
    using atomic_edge_allocator_traits =
        typename std::allocator_traits<allocator_type>::template rebind_traits<atomic_edge_t>;

    const vertex_size_type vertex_count = get_vertex_count_from_edge_list(edges);

    auto &graph_impl = oneapi::dal::detail::get_impl(g);
    auto &vertex_allocator = graph_impl._vertex_allocator;
    auto &edge_allocator = graph_impl._edge_allocator;
    atomic_vertex_allocator_type atomic_vertex_allocator;
    atomic_edge_allocator_type atomic_edge_allocator;

    atomic_vertex_t *degrees_cv =
        atomic_vertex_allocator_traits::allocate(atomic_vertex_allocator, vertex_count);
    degrees_cv = new (degrees_cv) atomic_vertex_t[vertex_count];
    //atomic_vertex_allocator_traits::construct(atomic_vertex_allocator, degrees_cv, 0);
    collect_degrees_from_edge_list(edges, degrees_cv);
    const vertex_size_type rows_vec_count = vertex_count + 1;
    if ((rows_vec_count - vertex_count) != static_cast<vertex_size_type>(1)) {
        throw range_error(dal::detail::error_messages::overflow_found_in_sum_of_two_values());
    }
    atomic_edge_t *rows_vec_atomic =
        atomic_edge_allocator_traits::allocate(atomic_edge_allocator, rows_vec_count);
    rows_vec_atomic = new (rows_vec_atomic) atomic_edge_t[rows_vec_count];

    edge_t total_sum_degrees =
        compute_prefix_sum_atomic<edge_t>(degrees_cv, vertex_count, rows_vec_atomic);

    atomic_vertex_allocator_traits::deallocate(atomic_vertex_allocator, degrees_cv, vertex_count);
    vertex_t *unfiltered_neighs =
        vertex_allocator_traits::allocate(vertex_allocator, total_sum_degrees);
    edge_t *unfiltered_offsets = edge_allocator_traits::allocate(edge_allocator, rows_vec_count);

    fill_from_atomics(unfiltered_offsets, rows_vec_atomic, rows_vec_count);
    fill_unfiltered_neighs(edges, rows_vec_atomic, unfiltered_neighs);

    atomic_edge_allocator_traits::deallocate(atomic_edge_allocator,
                                             rows_vec_atomic,
                                             rows_vec_count);
    vertex_t *degrees_data = vertex_allocator_traits::allocate(vertex_allocator, vertex_count);

    filter_neighbors_and_fill_new_degrees(unfiltered_neighs,
                                          unfiltered_offsets,
                                          degrees_data,
                                          vertex_count);

    edge_t *edge_offsets_data = edge_allocator_traits::allocate(edge_allocator, (vertex_count + 1));

    edge_t filtered_total_sum_degrees =
        compute_prefix_sum(degrees_data, vertex_count, edge_offsets_data);

    vertex_t *vertex_neighbors =
        vertex_allocator_traits::allocate(vertex_allocator, filtered_total_sum_degrees);

    fill_filtered_neighs(unfiltered_offsets,
                         unfiltered_neighs,
                         degrees_data,
                         edge_offsets_data,
                         vertex_neighbors,
                         vertex_count);

    vertex_allocator_traits::deallocate(vertex_allocator, unfiltered_neighs, total_sum_degrees);
    edge_allocator_traits::deallocate(edge_allocator, unfiltered_offsets, rows_vec_count);
    graph_impl.set_topology(vertex_count,
                            filtered_total_sum_degrees / 2,
                            edge_offsets_data,
                            vertex_neighbors,
                            degrees_data);
    return;
}

template <typename Descriptor, typename DataSource>
output_type<Descriptor> load_impl(const Descriptor &desc, const DataSource &data_source) {
    using graph_type = output_type<Descriptor>;
    graph_type graph;
    const auto el = load_edge_list<typename Descriptor::input_type::data_t::first_type>(
        data_source.get_filename());
    convert_to_csr_impl(el, graph);
    return graph;
}
} // namespace oneapi::dal::preview::load_graph::detail
