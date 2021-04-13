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

#include <initializer_list>

#include "oneapi/dal/algo/subgraph_isomorphism.hpp"
#include "oneapi/dal/graph/undirected_adjacency_vector_graph.hpp"
#include "oneapi/dal/table/common.hpp"
#include "oneapi/dal/exceptions.hpp"
#include "oneapi/dal/table/row_accessor.hpp"
#include "oneapi/dal/test/engine/common.hpp"

namespace oneapi::dal::algo::subgraph_isomorphism::test {

typedef dal::preview::subgraph_isomorphism::kind isomorphism_kind;

class subgraph_isomorphism_correctness_test {
public:
    auto create_graph_from_lil(const std::vector<std::vector<int>> &lil_graph) {
        oneapi::dal::preview::undirected_adjacency_vector_graph<std::int32_t> my_graph;
        auto &graph_impl = oneapi::dal::detail::get_impl(my_graph);
        auto &vertex_allocator = graph_impl._vertex_allocator;
        auto &edge_allocator = graph_impl._edge_allocator;

        const std::int64_t vertex_count = lil_graph.size();
        std::int64_t edge_count = 0;
        std::vector<int> prefix_sum(vertex_count + 1);
        for (int i = 0; i < vertex_count; ++i) {
            edge_count += lil_graph[i].size();
            prefix_sum[i + 1] = edge_count;
        }
        const std::int64_t cols_count = edge_count * 2;
        const std::int64_t rows_count = vertex_count + 1;

        std::int32_t *degrees_ =
            std::allocator_traits<std::allocator<char>>::rebind_traits<std::int32_t>::allocate(
                vertex_allocator,
                vertex_count);
        std::int32_t *cols_ =
            std::allocator_traits<std::allocator<char>>::rebind_traits<std::int32_t>::allocate(
                vertex_allocator,
                cols_count);
        std::int64_t *rows_ =
            std::allocator_traits<std::allocator<char>>::rebind_traits<std::int64_t>::allocate(
                edge_allocator,
                rows_count);
        std::int32_t *rows_vertex_ =
            std::allocator_traits<std::allocator<char>>::rebind_traits<std::int32_t>::allocate(
                vertex_allocator,
                rows_count);

        std::int32_t *degrees = new (degrees_) std::int32_t[vertex_count];
        std::int32_t *cols = new (cols_) std::int32_t[cols_count];
        std::int64_t *rows = new (rows_) std::int64_t[rows_count];
        std::int32_t *rows_vertex = new (rows_vertex_) std::int32_t[rows_count]{};

        int col_index = 0;
        for (int i = 0; i < vertex_count; ++i) {
            degrees[i] = lil_graph[i].size();
            for (int to : lil_graph[i]) {
                cols[col_index++] = to;
            }
        }
        for (int i = 0; i <= vertex_count; ++i) {
            rows[i] = prefix_sum[i];
            rows_vertex_[i] = prefix_sum[i];
        }

        graph_impl.set_topology(vertex_count, edge_count, rows, cols, degrees);
        graph_impl.get_topology()._rows_vertex =
            oneapi::dal::preview::detail::container<std::int32_t>::wrap(rows_vertex, rows_count);

        return my_graph;
    }

    bool check_isomorphism(const std::vector<int> &permutation,
                           const std::vector<std::vector<int>> &target,
                           const std::vector<std::vector<int>> &pattern) {
        std::vector<std::vector<int>> subgraph(permutation.size());
        std::map<int, int> reverse_permutation;
        for (size_t i = 0; i < permutation.size(); ++i) {
            reverse_permutation[permutation[i]] = i;
        }
        for (size_t i = 0; i < permutation.size(); ++i) {
            for (int j : target[permutation[i]]) {
                if (reverse_permutation.find(j) != reverse_permutation.end()) {
                    subgraph[i].push_back(reverse_permutation[j]);
                }
            }
            std::sort(subgraph[i].begin(), subgraph[i].end());
        }
        return subgraph == pattern;
    }

    auto graph_matching_wrapper(const std::vector<std::vector<int>> &target,
                                const std::vector<std::vector<int>> &pattern,
                                bool semantic_match,
                                std::int64_t max_match_count,
                                isomorphism_kind kind) {
        auto target_graph = this->create_graph_from_lil(target);
        auto pattern_graph = this->create_graph_from_lil(pattern);

        std::allocator<char> alloc;
        const auto subgraph_isomorphism_desc =
            dal::preview::subgraph_isomorphism::descriptor<>(alloc)
                .set_kind(kind)
                .set_semantic_match(semantic_match)
                .set_max_match_count(max_match_count);

        return dal::preview::graph_matching(subgraph_isomorphism_desc, target_graph, pattern_graph);
    }

    bool check_graph_isomorphism_correctness(const oneapi::dal::table &table,
                                             std::vector<std::vector<int>> &target,
                                             std::vector<std::vector<int>> &pattern) {
        if (!table.has_data())
            return true;
        auto arr = oneapi::dal::row_accessor<const int>(table).pull();
        const auto x = arr.get_data();

        for (std::int64_t i = 0; i < table.get_row_count(); i++) {
            std::vector<int> permutation(table.get_column_count());
            for (std::int64_t j = 0; j < table.get_column_count(); j++) {
                permutation[j] = x[i * table.get_column_count() + j];
            }
            if (!check_isomorphism(permutation, target, pattern)) {
                return false;
            }
        }
        return true;
    }
};

#define SUBGRAPH_ISOMORPHISM_CORRECTNESS_TEST(name) \
    TEST_M(subgraph_isomorphism_correctness_test, name, "[subgraph_isomorphism][correctness]")

std::vector<std::vector<int>> linked_star_5 = { { 1, 4 }, { 0, 2 }, { 1, 3 }, { 2, 4 }, { 3, 0 } };
std::vector<std::vector<int>> cycle_5 = { { 2, 3 }, { 3, 4 }, { 0, 4 }, { 0, 1 }, { 1, 2 } };
std::vector<std::vector<int>> self_matching = { { 1 }, { 0, 2, 5, 10 }, { 1, 3, 6, 9 },  { 2 },
                                                { 5 }, { 1, 4, 6, 9 },  { 2, 5, 7, 10 }, { 6 },
                                                { 9 }, { 2, 5, 8, 10 }, { 1, 6, 9, 11 }, { 10 } };
std::vector<std::vector<int>> double_triangle_target = {
    { 1, 2, 3 },       { 0, 2, 3, 5 }, { 0, 1, 3, 4 },    { 0, 1, 2, 7 }, { 2, 5, 6, 7 },
    { 1, 4, 6, 7, 8 }, { 4, 5, 7 },    { 3, 4, 5, 6, 8 }, { 5, 7 }
};
std::vector<std::vector<int>> double_triangle_pattern = { { 1, 3 },
                                                          { 0, 2, 3 },
                                                          { 1, 3 },
                                                          { 0, 1, 2 } };
std::vector<std::vector<int>> k_6 = { { 1, 2, 3, 4, 5 }, { 0, 2, 3, 4, 5 }, { 0, 1, 3, 4, 5 },
                                      { 0, 1, 2, 4, 5 }, { 0, 1, 2, 3, 5 }, { 0, 1, 2, 3, 4 } };
std::vector<std::vector<int>> k_5_without_edge = { { 1, 2, 4 },
                                                   { 0, 2, 3, 4 },
                                                   { 0, 1, 3, 4 },
                                                   { 1, 2, 4 },
                                                   { 0, 1, 2, 3 } };
std::vector<std::vector<int>> difficult_graph = {
    { 1, 2, 3 },        { 0, 2 },          { 0, 1 },       { 0, 4, 5, 6, 7, 8 },
    { 3, 5, 6, 7 },     { 3, 4, 6, 7 },    { 3, 4, 5, 7 }, { 3, 4, 5, 6 },
    { 3, 9, 10, 11 },   { 8, 10, 11, 13 }, { 8, 9, 11 },   { 8, 9, 10, 12 },
    { 11, 13, 14, 15 }, { 9, 12, 14, 15 }, { 12, 13, 15 }, { 12, 13, 14 }
};
std::vector<std::vector<int>> triagles_edge_link = { { 1, 2 },    { 0, 2 }, { 0, 1, 3 },
                                                     { 2, 4, 5 }, { 3, 5 }, { 3, 4 } };
std::vector<std::vector<int>> star_5 = { { 1, 2, 3, 4, 5 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 } };
std::vector<std::vector<int>> star_4 = { { 1, 2, 3, 4 }, { 0 }, { 0 }, { 0 }, { 0 } };
std::vector<std::vector<int>> wheel_11 = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
                                           { 2, 10 },
                                           { 1, 3 },
                                           { 2, 4 },
                                           { 3, 5 },
                                           { 4, 6 },
                                           { 5, 7 },
                                           { 6, 8 },
                                           { 7, 9 },
                                           { 8, 10 },
                                           { 1, 9 } };
std::vector<std::vector<int>> cycle_10 = { { 1, 9 }, { 0, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 },
                                           { 4, 6 }, { 5, 7 }, { 6, 8 }, { 7, 9 }, { 0, 8 } };
std::vector<std::vector<int>> wheel_5 = { { 1, 2, 3, 4 },
                                          { 0, 2, 4 },
                                          { 0, 1, 3 },
                                          { 0, 2, 4 },
                                          { 0, 1, 3 } };
std::vector<std::vector<int>> triangle = { { 1, 2 }, { 0, 2 }, { 0, 1 } };
std::vector<std::vector<int>> lolipop_10_15 = { { 1, 2, 3, 4, 5, 6, 7, 8, 9 },
                                                { 0, 2, 3, 4, 5, 6, 7, 8, 9 },
                                                { 0, 1, 3, 4, 5, 6, 7, 8, 9 },
                                                { 0, 1, 2, 4, 5, 6, 7, 8, 9 },
                                                { 0, 1, 2, 3, 5, 6, 7, 8, 9 },
                                                { 0, 1, 2, 3, 4, 6, 7, 8, 9 },
                                                { 0, 1, 2, 3, 4, 5, 7, 8, 9 },
                                                { 0, 1, 2, 3, 4, 5, 6, 8, 9 },
                                                { 0, 1, 2, 3, 4, 5, 6, 7, 9 },
                                                { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 },
                                                { 9, 11 },
                                                { 10, 12 },
                                                { 11, 13 },
                                                { 12, 14 },
                                                { 13, 15 },
                                                { 14, 16 },
                                                { 15, 17 },
                                                { 16, 18 },
                                                { 17, 19 },
                                                { 18, 20 },
                                                { 19, 21 },
                                                { 20, 22 },
                                                { 21, 23 },
                                                { 22, 24 },
                                                { 23 } };
std::vector<std::vector<int>> path_16 = { { 1 },      { 0, 2 },   { 1, 3 },   { 2, 4 },
                                          { 3, 5 },   { 4, 6 },   { 5, 7 },   { 6, 8 },
                                          { 7, 9 },   { 8, 10 },  { 9, 11 },  { 10, 12 },
                                          { 11, 13 }, { 12, 14 }, { 13, 15 }, { 14 } };

SUBGRAPH_ISOMORPHISM_CORRECTNESS_TEST("linked_star_5  - cycle_5 matching") {
    const auto result =
        this->graph_matching_wrapper(linked_star_5, cycle_5, false, 0, isomorphism_kind::induced);
    REQUIRE(10 == result.get_match_count());
    REQUIRE(check_graph_isomorphism_correctness(result.get_vertex_match(), linked_star_5, cycle_5));
}

SUBGRAPH_ISOMORPHISM_CORRECTNESS_TEST("self_matching") {
    const auto result = this->graph_matching_wrapper(self_matching,
                                                     self_matching,
                                                     false,
                                                     0,
                                                     isomorphism_kind::induced);
    REQUIRE(72 == result.get_match_count());
    REQUIRE(check_graph_isomorphism_correctness(result.get_vertex_match(),
                                                self_matching,
                                                self_matching));
}

SUBGRAPH_ISOMORPHISM_CORRECTNESS_TEST("double_tringle matching") {
    const auto result = this->graph_matching_wrapper(double_triangle_target,
                                                     double_triangle_pattern,
                                                     false,
                                                     0,
                                                     isomorphism_kind::induced);
    REQUIRE(8 == result.get_match_count());
    REQUIRE(check_graph_isomorphism_correctness(result.get_vertex_match(),
                                                double_triangle_target,
                                                double_triangle_pattern));
}

SUBGRAPH_ISOMORPHISM_CORRECTNESS_TEST("k_6 - k_5_without_edge matching") {
    const auto result =
        this->graph_matching_wrapper(k_6, k_5_without_edge, false, 0, isomorphism_kind::induced);
    REQUIRE(0 == result.get_match_count());
    // REQUIRE(check_graph_isomorphism_correctness(result.get_vertex_match(), k_6, k_5_without_edge));
}

SUBGRAPH_ISOMORPHISM_CORRECTNESS_TEST("difficult matching") {
    const auto result = this->graph_matching_wrapper(difficult_graph,
                                                     triagles_edge_link,
                                                     false,
                                                     0,
                                                     isomorphism_kind::induced);
    REQUIRE(272 == result.get_match_count());
    REQUIRE(check_graph_isomorphism_correctness(result.get_vertex_match(),
                                                difficult_graph,
                                                triagles_edge_link));
}

SUBGRAPH_ISOMORPHISM_CORRECTNESS_TEST("star_5 - star_4 matching") {
    const auto result =
        this->graph_matching_wrapper(star_5, star_4, false, 0, isomorphism_kind::induced);
    REQUIRE(120 == result.get_match_count());
    REQUIRE(check_graph_isomorphism_correctness(result.get_vertex_match(), star_5, star_4));
}

SUBGRAPH_ISOMORPHISM_CORRECTNESS_TEST("wheel_11 - cycle_10 matching") {
    const auto result =
        this->graph_matching_wrapper(wheel_11, cycle_10, false, 0, isomorphism_kind::induced);
    REQUIRE(20 == result.get_match_count());
    REQUIRE(check_graph_isomorphism_correctness(result.get_vertex_match(), wheel_11, cycle_10));
}

SUBGRAPH_ISOMORPHISM_CORRECTNESS_TEST("wheel_5 - triangle matching") {
    const auto result =
        this->graph_matching_wrapper(wheel_5, triangle, false, 0, isomorphism_kind::induced);
    REQUIRE(24 == result.get_match_count());
    REQUIRE(check_graph_isomorphism_correctness(result.get_vertex_match(), wheel_5, triangle));
}

SUBGRAPH_ISOMORPHISM_CORRECTNESS_TEST("lolipop_10_15 - path_16 matching") {
    const auto result =
        this->graph_matching_wrapper(lolipop_10_15, path_16, false, 0, isomorphism_kind::induced);
    REQUIRE(20 == result.get_match_count());
    REQUIRE(check_graph_isomorphism_correctness(result.get_vertex_match(), lolipop_10_15, path_16));
}

} // namespace oneapi::dal::algo::subgraph_isomorphism::test
