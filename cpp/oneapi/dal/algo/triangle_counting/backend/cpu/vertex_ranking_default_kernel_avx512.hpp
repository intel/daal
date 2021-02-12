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

#include <immintrin.h>
#include <functional>

#include <daal/src/services/service_defines.h>

#include "oneapi/dal/algo/triangle_counting/backend/cpu/vertex_ranking_default_kernel.hpp"
#include "oneapi/dal/algo/triangle_counting/common.hpp"
#include "oneapi/dal/algo/triangle_counting/vertex_ranking_types.hpp"
#include "oneapi/dal/backend/dispatcher.hpp"
#include "oneapi/dal/backend/interop/common.hpp"
#include "oneapi/dal/backend/interop/table_conversion.hpp"
#include "oneapi/dal/detail/policy.hpp"
#include "oneapi/dal/detail/threading.hpp"
#include "oneapi/dal/graph/detail/service_functions_impl.hpp"
#include "oneapi/dal/table/detail/table_builder.hpp"
#include <iostream>

namespace oneapi::dal::preview {
namespace triangle_counting {
namespace detail {

#if defined(__INTEL_COMPILER)
DAAL_FORCEINLINE std::int32_t _popcnt32_redef(const std::int32_t &x) {
    return _popcnt32(x);
}
#define GRAPH_STACK_ALING(x) __declspec(align(x))
#else
DAAL_FORCEINLINE std::int32_t _popcnt32_redef(const std::int32_t &x) {
    std::int32_t count = 0;
    std::int32_t a = x;
    while (a != 0) {
        a = a & (a - 1);
        count++;
    }
    return count;
}
#define GRAPH_STACK_ALING(x) \
    {}
#endif

DAAL_FORCEINLINE std::int64_t intersection(const std::int32_t *neigh_u,
                                           const std::int32_t *neigh_v,
                                           std::int32_t n_u,
                                           std::int32_t n_v) {
    std::int64_t total = 0;
    std::int32_t i_u = 0, i_v = 0;
#if defined(__INTEL_COMPILER)
    while (i_u < (n_u / 16) * 16 && i_v < (n_v / 16) * 16) { // not in last n%16 elements
        // assumes neighbor list is ordered
        std::int32_t min_neigh_u = neigh_u[i_u];
        std::int32_t max_neigh_v = neigh_v[i_v + 15];

        if (min_neigh_u > max_neigh_v) {
            if (min_neigh_u > neigh_v[n_v - 1]) {
                return total;
            }
            i_v += 16;
            continue;
        }

        std::int32_t min_neigh_v = neigh_v[i_v];
        std::int32_t max_neigh_u = neigh_u[i_u + 15];
        if (min_neigh_v > max_neigh_u) {
            if (min_neigh_v > neigh_u[n_u - 1]) {
                return total;
            }
            i_u += 16;
            continue;
        }
        __m512i v_u = _mm512_loadu_si512((void *)(neigh_u + i_u)); // load 16 neighbors of u
        __m512i v_v = _mm512_loadu_si512((void *)(neigh_v + i_v)); // load 16 neighbors of v
        if (max_neigh_u >= max_neigh_v)
            i_v += 16;
        if (max_neigh_u <= max_neigh_v)
            i_u += 16;

        __mmask16 match = _mm512_cmpeq_epi32_mask(v_u, v_v);
        if (_mm512_mask2int(match) != 0xffff) { // shortcut case where all neighbors match
            __m512i circ1 = _mm512_set_epi32(0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
            __m512i circ2 = _mm512_set_epi32(1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2);
            __m512i circ3 = _mm512_set_epi32(2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3);
            __m512i circ4 = _mm512_set_epi32(3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4);
            __m512i circ5 = _mm512_set_epi32(4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5);
            __m512i circ6 = _mm512_set_epi32(5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6);
            __m512i circ7 = _mm512_set_epi32(6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7);
            __m512i circ8 = _mm512_set_epi32(7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8);
            __m512i circ9 = _mm512_set_epi32(8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9);
            __m512i circ10 = _mm512_set_epi32(9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10);
            __m512i circ11 = _mm512_set_epi32(10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11);
            __m512i circ12 = _mm512_set_epi32(11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12);
            __m512i circ13 = _mm512_set_epi32(12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13);
            __m512i circ14 = _mm512_set_epi32(13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14);
            __m512i circ15 = _mm512_set_epi32(14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15);
            __m512i v_v1 = _mm512_permutexvar_epi32(circ1, v_v);
            __m512i v_v2 = _mm512_permutexvar_epi32(circ2, v_v);
            __m512i v_v3 = _mm512_permutexvar_epi32(circ3, v_v);
            __m512i v_v4 = _mm512_permutexvar_epi32(circ4, v_v);
            __m512i v_v5 = _mm512_permutexvar_epi32(circ5, v_v);
            __m512i v_v6 = _mm512_permutexvar_epi32(circ6, v_v);
            __m512i v_v7 = _mm512_permutexvar_epi32(circ7, v_v);
            __m512i v_v8 = _mm512_permutexvar_epi32(circ8, v_v);
            __m512i v_v9 = _mm512_permutexvar_epi32(circ9, v_v);
            __m512i v_v10 = _mm512_permutexvar_epi32(circ10, v_v);
            __m512i v_v11 = _mm512_permutexvar_epi32(circ11, v_v);
            __m512i v_v12 = _mm512_permutexvar_epi32(circ12, v_v);
            __m512i v_v13 = _mm512_permutexvar_epi32(circ13, v_v);
            __m512i v_v14 = _mm512_permutexvar_epi32(circ14, v_v);
            __m512i v_v15 = _mm512_permutexvar_epi32(circ15, v_v);
            __mmask16 tmp_match1 = _mm512_cmpeq_epi32_mask(v_u, v_v1); // find matches
            __mmask16 tmp_match2 = _mm512_cmpeq_epi32_mask(v_u, v_v2);
            __mmask16 tmp_match3 = _mm512_cmpeq_epi32_mask(v_u, v_v3);
            __mmask16 tmp_match4 = _mm512_cmpeq_epi32_mask(v_u, v_v4);
            __mmask16 tmp_match5 = _mm512_cmpeq_epi32_mask(v_u, v_v5);
            __mmask16 tmp_match6 = _mm512_cmpeq_epi32_mask(v_u, v_v6);
            __mmask16 tmp_match7 = _mm512_cmpeq_epi32_mask(v_u, v_v7);
            __mmask16 tmp_match8 = _mm512_cmpeq_epi32_mask(v_u, v_v8);
            __mmask16 tmp_match9 = _mm512_cmpeq_epi32_mask(v_u, v_v9);
            __mmask16 tmp_match10 = _mm512_cmpeq_epi32_mask(v_u, v_v10);
            __mmask16 tmp_match11 = _mm512_cmpeq_epi32_mask(v_u, v_v11);
            __mmask16 tmp_match12 = _mm512_cmpeq_epi32_mask(v_u, v_v12);
            __mmask16 tmp_match13 = _mm512_cmpeq_epi32_mask(v_u, v_v13);
            __mmask16 tmp_match14 = _mm512_cmpeq_epi32_mask(v_u, v_v14);
            __mmask16 tmp_match15 = _mm512_cmpeq_epi32_mask(v_u, v_v15);
            match = _mm512_kor(
                _mm512_kor(
                    _mm512_kor(_mm512_kor(match, tmp_match1), _mm512_kor(tmp_match2, tmp_match3)),
                    _mm512_kor(_mm512_kor(tmp_match4, tmp_match5),
                               _mm512_kor(tmp_match6, tmp_match7))),
                _mm512_kor(
                    _mm512_kor(_mm512_kor(tmp_match8, tmp_match9),
                               _mm512_kor(tmp_match10, tmp_match11)),
                    _mm512_kor(_mm512_kor(tmp_match12, tmp_match13),
                               _mm512_kor(tmp_match14, tmp_match15)))); // combine all matches
        }
        total += _popcnt32_redef(_mm512_mask2int(match)); //count number of matches
    }

    while (i_u < (n_u / 16) * 16 && i_v < n_v) {
        __m512i v_u = _mm512_loadu_si512((void *)(neigh_u + i_u));
        while (neigh_v[i_v] <= neigh_u[i_u + 15] && i_v < n_v) {
            __m512i tmp_v_v = _mm512_set1_epi32(neigh_v[i_v]);
            __mmask16 match = _mm512_cmpeq_epi32_mask(v_u, tmp_v_v);
            if (_mm512_mask2int(match))
                total++;
            i_v++;
        }
        i_u += 16;
    }
    while (i_v < (n_v / 16) * 16 && i_u < n_u) {
        __m512i v_v = _mm512_loadu_si512((void *)(neigh_v + i_v));
        while (neigh_u[i_u] <= neigh_v[i_v + 15] && i_u < n_u) {
            __m512i tmp_v_u = _mm512_set1_epi32(neigh_u[i_u]);
            __mmask16 match = _mm512_cmpeq_epi32_mask(v_v, tmp_v_u);
            if (_mm512_mask2int(match))
                total++;
            i_u++;
        }
        i_v += 16;
    }

    while (i_u <= (n_u - 8) && i_v <= (n_v - 8)) { // not in last n%8 elements
        // assumes neighbor list is ordered
        std::int32_t min_neigh_u = neigh_u[i_u];
        std::int32_t max_neigh_v = neigh_v[i_v + 7];

        if (min_neigh_u > max_neigh_v) {
            if (min_neigh_u > neigh_v[n_v - 1]) {
                return total;
            }
            i_v += 8;
            continue;
        }
        std::int32_t max_neigh_u = neigh_u[i_u + 7];
        std::int32_t min_neigh_v = neigh_v[i_v];
        if (min_neigh_v > max_neigh_u) {
            if (min_neigh_v > neigh_u[n_u - 1]) {
                return total;
            }
            i_u += 8;
            continue;
        }
        __m256i v_u = _mm256_loadu_si256(
            reinterpret_cast<const __m256i *>(neigh_u + i_u)); // load 8 neighbors of u
        __m256i v_v = _mm256_loadu_si256(
            reinterpret_cast<const __m256i *>(neigh_v + i_v)); // load 8 neighbors of v

        if (max_neigh_u >= max_neigh_v)
            i_v += 8;
        if (max_neigh_u <= max_neigh_v)
            i_u += 8;

        __mmask8 match = _mm256_cmpeq_epi32_mask(v_u, v_v);
        if (_cvtmask8_u32(match) != 0xff) { // shortcut case where all neighbors match
            __m256i circ1 = _mm256_set_epi32(0, 7, 6, 5, 4, 3, 2, 1);
            __m256i circ2 = _mm256_set_epi32(1, 0, 7, 6, 5, 4, 3, 2);
            __m256i circ3 = _mm256_set_epi32(2, 1, 0, 7, 6, 5, 4, 3);
            __m256i circ4 = _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4);
            __m256i circ5 = _mm256_set_epi32(4, 3, 2, 1, 0, 7, 6, 5);
            __m256i circ6 = _mm256_set_epi32(5, 4, 3, 2, 1, 0, 7, 6);
            __m256i circ7 = _mm256_set_epi32(6, 5, 4, 3, 2, 1, 0, 7);

            __m256i v_v1 = _mm256_permutexvar_epi32(circ1, v_v);
            __m256i v_v2 = _mm256_permutexvar_epi32(circ2, v_v);
            __m256i v_v3 = _mm256_permutexvar_epi32(circ3, v_v);
            __m256i v_v4 = _mm256_permutexvar_epi32(circ4, v_v);
            __m256i v_v5 = _mm256_permutexvar_epi32(circ5, v_v);
            __m256i v_v6 = _mm256_permutexvar_epi32(circ6, v_v);
            __m256i v_v7 = _mm256_permutexvar_epi32(circ7, v_v);

            __mmask8 tmp_match1 = _mm256_cmpeq_epi32_mask(v_u, v_v1); // find matches
            __mmask8 tmp_match2 = _mm256_cmpeq_epi32_mask(v_u, v_v2);
            __mmask8 tmp_match3 = _mm256_cmpeq_epi32_mask(v_u, v_v3);
            __mmask8 tmp_match4 = _mm256_cmpeq_epi32_mask(v_u, v_v4);
            __mmask8 tmp_match5 = _mm256_cmpeq_epi32_mask(v_u, v_v5);
            __mmask8 tmp_match6 = _mm256_cmpeq_epi32_mask(v_u, v_v6);
            __mmask8 tmp_match7 = _mm256_cmpeq_epi32_mask(v_u, v_v7);

            match = _kor_mask8(
                _kor_mask8(_kor_mask8(match, tmp_match1), _kor_mask8(tmp_match2, tmp_match3)),
                _kor_mask8(_kor_mask8(tmp_match4, tmp_match5),
                           _kor_mask8(tmp_match6, tmp_match7))); // combine all matches
        }
        total += _popcnt32_redef(_cvtmask8_u32(match)); //count number of matches
    }
    if (i_u <= (n_u - 8) && i_v < n_v) {
        __m256i v_u = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(neigh_u + i_u));
        while (neigh_v[i_v] <= neigh_u[i_u + 7] && i_v < n_v) {
            __m256i tmp_v_v = _mm256_set1_epi32(neigh_v[i_v]);
            __mmask8 match = _mm256_cmpeq_epi32_mask(v_u, tmp_v_v);
            if (_cvtmask8_u32(match))
                total++;
            i_v++;
        }
        i_u += 8;
    }
    if (i_v <= (n_v - 8) && i_u < n_u) {
        __m256i v_v = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(neigh_v + i_v));
        while (neigh_u[i_u] <= neigh_v[i_v + 7] && i_u < n_u) {
            __m256i tmp_v_u = _mm256_set1_epi32(neigh_u[i_u]);
            __mmask8 match = _mm256_cmpeq_epi32_mask(v_v, tmp_v_u);
            if (_cvtmask8_u32(match))
                total++;
            i_u++;
        }
        i_v += 8;
    }

    while (i_u <= (n_u - 4) && i_v <= (n_v - 4)) { // not in last n%8 elements
        // assumes neighbor list is ordered
        std::int32_t min_neigh_u = neigh_u[i_u];
        std::int32_t max_neigh_v = neigh_v[i_v + 3];

        if (min_neigh_u > max_neigh_v) {
            if (min_neigh_u > neigh_v[n_v - 1]) {
                return total;
            }
            i_v += 4;
            continue;
        }
        std::int32_t min_neigh_v = neigh_v[i_v];
        std::int32_t max_neigh_u = neigh_u[i_u + 3];
        if (min_neigh_v > max_neigh_u) {
            if (min_neigh_v > neigh_u[n_u - 1]) {
                return total;
            }
            i_u += 4;
            continue;
        }
        __m128i v_u = _mm_loadu_si128(
            reinterpret_cast<const __m128i *>(neigh_u + i_u)); // load 8 neighbors of u
        __m128i v_v = _mm_loadu_si128(
            reinterpret_cast<const __m128i *>(neigh_v + i_v)); // load 8 neighbors of v

        if (max_neigh_u >= max_neigh_v)
            i_v += 4;
        if (max_neigh_u <= max_neigh_v)
            i_u += 4;

        __mmask8 match = _mm_cmpeq_epi32_mask(v_u, v_v);
        if (_cvtmask8_u32(match) != 0xf) { // shortcut case where all neighbors match
            __m128i v_v1 = _mm_shuffle_epi32(v_v, _MM_SHUFFLE(0, 3, 2, 1));
            __m128i v_v2 = _mm_shuffle_epi32(v_v, _MM_SHUFFLE(1, 0, 3, 2));
            __m128i v_v3 = _mm_shuffle_epi32(v_v, _MM_SHUFFLE(2, 1, 0, 3));

            __mmask8 tmp_match1 = _mm_cmpeq_epi32_mask(v_u, v_v1); // find matches
            __mmask8 tmp_match2 = _mm_cmpeq_epi32_mask(v_u, v_v2);
            __mmask8 tmp_match3 = _mm_cmpeq_epi32_mask(v_u, v_v3);

            match = _kor_mask8(_kor_mask8(match, tmp_match1),
                               _kor_mask8(tmp_match2, tmp_match3)); // combine all matches
        }
        total += _popcnt32_redef(_cvtmask8_u32(match)); //count number of matches
    }
    if (i_u <= (n_u - 4) && i_v < n_v) {
        __m128i v_u = _mm_loadu_si128(reinterpret_cast<const __m128i *>(neigh_u + i_u));
        while (neigh_v[i_v] <= neigh_u[i_u + 3] && i_v < n_v) {
            __m128i tmp_v_v = _mm_set1_epi32(neigh_v[i_v]);
            __mmask8 match = _mm_cmpeq_epi32_mask(v_u, tmp_v_v);
            if (_cvtmask8_u32(match))
                total++;
            i_v++;
        }
        i_u += 4;
    }
    if (i_v <= (n_v - 4) && i_u < n_u) {
        __m128i v_v = _mm_loadu_si128(reinterpret_cast<const __m128i *>(neigh_v + i_v));
        while (neigh_u[i_u] <= neigh_v[i_v + 3] && i_u < n_u) {
            __m128i tmp_v_u = _mm_set1_epi32(neigh_u[i_u]);
            __mmask8 match = _mm_cmpeq_epi32_mask(v_v, tmp_v_u);
            if (_cvtmask8_u32(match))
                total++;
            i_u++;
        }
        i_v += 4;
    }
#endif
    while (i_u < n_u && i_v < n_v) {
        if ((neigh_u[i_u] > neigh_v[n_v - 1]) || (neigh_v[i_v] > neigh_u[n_u - 1])) {
            return total;
        }
        if (neigh_u[i_u] == neigh_v[i_v])
            total++, i_u++, i_v++;
        else if (neigh_u[i_u] < neigh_v[i_v])
            i_u++;
        else if (neigh_u[i_u] > neigh_v[i_v])
            i_v++;
    }
    return total;
}

template <typename Cpu>
vertex_ranking_result<task::local> call_triangle_counting_default_kernel_avx512(
    const detail::descriptor_base<task::local> &desc,
    const dal::preview::detail::topology<std::int32_t> &data) {
    std::cout << "local tc avx512" << std::endl;

    vertex_ranking_result<task::local> res;
    return res;
}

std::int64_t triangle_counting_(const std::int32_t* vertex_neighbors, const std::int64_t* edge_offsets, 
                                const std::int32_t* degrees, std::int64_t vertex_count, std::int64_t edge_count) {
    std::int32_t average_degree = edge_count / vertex_count;
    const std::int32_t average_degree_sparsity_boundary = 4;
    if (average_degree < average_degree_sparsity_boundary) {
        std::int64_t total_s = oneapi::dal::detail::parallel_reduce_size_t_int64_t(vertex_count, (std::int64_t)0,
            [&] (std::int64_t begin_u, std::int64_t end_u, std::int64_t tc_u) -> std::int64_t {
                for (auto u = begin_u; u!= end_u; ++u) {
                    for (auto v_ = vertex_neighbors + edge_offsets[u]; v_ != vertex_neighbors + edge_offsets[u+1]; ++v_) {
                        std::int32_t v = *v_;
                        if (v > u) {
                            break;
                        }
                        auto u_neighbors_ptr = vertex_neighbors + edge_offsets[u];
                        for (auto w_ = vertex_neighbors + edge_offsets[v]; v_ != vertex_neighbors + edge_offsets[v+1]; ++w_) {
                            std::int32_t w = *w_;
                            if (w > v){
                                break;
                            }
                            while (*u_neighbors_ptr < w){
                                u_neighbors_ptr++;
                            }
                            if (w == *u_neighbors_ptr){
                                tc_u++;
                            }
                        }
                    }
                }
                return tc_u;
            },
            [&](std::int64_t x, std::int64_t y) -> std::int64_t  {
                return x + y;
            });
        return total_s;
    } else {
            std::int64_t total_s = oneapi::dal::detail::parallel_reduce_size_t_int64_t_simple(vertex_count, (std::int64_t)0,
            [&] (std::int64_t begin_u, std::int64_t end_u, std::int64_t tc_u) -> std::int64_t {
                for (auto u = begin_u; u!= end_u; ++u) {                                     
                    if (degrees[u] < 2){
                        continue; 
                    }
                    const std::int32_t* neigh_u = vertex_neighbors + edge_offsets[u];
                    std::int32_t size_neigh_u = vertex_neighbors + edge_offsets[u+1] - neigh_u;

                    tc_u += oneapi::dal::detail::parallel_reduce_int32ptr_int64_t_simple(vertex_neighbors + edge_offsets[u], vertex_neighbors + edge_offsets[u+1], (std::int64_t)0,
                            [&] (const std::int32_t* begin_v, const std::int32_t* end_v, std::int64_t total) -> std::int64_t {
                                for (auto v_ = begin_v; v_!= end_v; ++v_) {
                    
                                std::int32_t v = *v_;

                                if (v > u) {
                                    break;
                                }

                                const std::int32_t* neigh_v = vertex_neighbors + edge_offsets[v];
                                std::int32_t size_neigh_v = vertex_neighbors + edge_offsets[v+1] - neigh_v;   

                                std::int32_t new_size_neigh_v = 0;
                                for (new_size_neigh_v = 0; (new_size_neigh_v < size_neigh_v) && (neigh_v[new_size_neigh_v] <= v); new_size_neigh_v++);
                                

                                total += intersection(neigh_u, neigh_v, size_neigh_u, new_size_neigh_v);
                            }
                            return total;
                        },
                        [&](std::int64_t x, std::int64_t y) -> std::int64_t  {
                            return x + y;
                        });
                }     
                return tc_u;
            },
            [&](std::int64_t x, std::int64_t y) -> std::int64_t  {
                return x + y;
            });
        return total_s;
    }
}

template <typename Cpu>
vertex_ranking_result<task::global> call_triangle_counting_default_kernel_avx512(
    const detail::descriptor_base<task::global> &desc,
    const dal::preview::detail::topology<std::int32_t> &data) {
    std::cout << "global tc avx512" << std::endl;
    const auto g_edge_offsets = data._rows.get_data();
    const auto g_vertex_neighbors = data._cols.get_data();
    const auto g_degrees = data._degrees.get_data();
    const auto g_vertex_count = data._vertex_count;
    const auto g_edge_count = data._edge_count;

    const auto relabel = desc.get_relabel();
    std::int64_t triangles = 0;

    const std::int32_t average_degree_sparsity_boundary = 4;
    if (g_edge_count / g_vertex_count > average_degree_sparsity_boundary && relabel == relabel::yes) {
        std::int32_t* g_vertex_neighbors_relabel = nullptr;
        std::int64_t* g_edge_offsets_relabel = nullptr;
        std::int32_t* g_degrees_relabel = nullptr;
        relabel_by_greater_degree(g_vertex_neighbors, g_edge_offsets, g_degrees, g_vertex_count, g_edge_count,
                                  g_vertex_neighbors_relabel, g_edge_offsets_relabel, g_degrees_relabel);

    }
    else {

        triangles = triangle_counting_(g_vertex_neighbors, g_edge_offsets, g_degrees, g_vertex_count, g_edge_count);
    }

    vertex_ranking_result<task::global> res;
    res.set_global_rank(triangles);
    return res;
}
} // namespace detail
} // namespace triangle_counting
} // namespace oneapi::dal::preview