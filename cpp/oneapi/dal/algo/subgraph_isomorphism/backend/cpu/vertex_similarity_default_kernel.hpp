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

#include <memory>

#include "oneapi/dal/algo/subgraph_isomorphism/common.hpp"
#include "oneapi/dal/algo/subgraph_isomorphism/graph_matching_types.hpp"
#include "oneapi/dal/backend/dispatcher.hpp"
#include "oneapi/dal/backend/interop/common.hpp"
#include "oneapi/dal/common.hpp"
#include "oneapi/dal/detail/policy.hpp"

namespace oneapi::dal::preview {
namespace subgraph_isomorphism {
namespace detail {

template <typename Cpu>
graph_matching_result call_subgraph_isomorphism_default_kernel_int32(
    const descriptor_base &desc,
    const dal::preview::detail::topology<int32_t> &data,
    void *result_ptr);

DAAL_FORCEINLINE std::int32_t min(const std::int32_t &a, const std::int32_t &b) {
    return (a >= b) ? b : a;
}

DAAL_FORCEINLINE std::int32_t max(const std::int32_t &a, const std::int32_t &b) {
    return (a <= b) ? b : a;
}

DAAL_FORCEINLINE std::int64_t compute_number_elements_in_block(
    const std::int32_t &row_range_begin,
    const std::int32_t &row_range_end,
    const std::int32_t &column_range_begin,
    const std::int32_t &column_range_end) {
    ONEDAL_ASSERT(row_range_end >= row_range_begin, "Negative interval found");
    const std::int64_t row_count = row_range_end - row_range_begin;
    ONEDAL_ASSERT(column_range_end >= column_range_begin, "Negative interval found");
    const std::int64_t column_count = column_range_end - column_range_begin;
    // compute the number of the vertex pairs in the block of the graph
    const std::int64_t vertex_pairs_count = row_count * column_count;
    ONEDAL_ASSERT(vertex_pairs_count / row_count == column_count,
                  "Overflow found in multiplication of two values");
    return vertex_pairs_count;
}

template <typename Float, typename Index>
DAAL_FORCEINLINE std::int64_t compute_max_block_size(const std::int64_t &vertex_pairs_count) {
    const std::int64_t vertex_pair_element_count = 2; // 2 elements in the vertex pair
    const std::int64_t subgraph_isomorphism_coeff_element_count =
        1; // 1 subgraph_isomorphism coeff for the vertex pair

    const std::int64_t vertex_pair_size =
        vertex_pair_element_count * sizeof(Index); // size in bytes
    const std::int64_t subgraph_isomorphism_coeff_size =
        subgraph_isomorphism_coeff_element_count * sizeof(Float); // size in bytes
    const std::int64_t element_result_size = vertex_pair_size + subgraph_isomorphism_coeff_size;

    const std::int64_t block_result_size = element_result_size * vertex_pairs_count;
    ONEDAL_ASSERT(block_result_size / vertex_pairs_count == element_result_size,
                  "Overflow found in multiplication of two values");
    return block_result_size;
}

} // namespace detail
} // namespace subgraph_isomorphism
} // namespace oneapi::dal::preview
