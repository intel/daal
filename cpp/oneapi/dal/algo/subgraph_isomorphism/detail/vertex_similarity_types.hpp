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

/// @file
/// Contains the definition of the input and output for subgraph_isomorphism Similarity
/// algorithm

#pragma once

#include "oneapi/dal/algo/subgraph_isomorphism/common.hpp"

namespace oneapi::dal::preview::subgraph_isomorphism::detail {

class graph_matching_result_impl;

template <typename Graph>
class graph_matching_input_impl : public base {
public:
    graph_matching_input_impl(const Graph& graph_data_input, caching_builder& builder_input)
            : graph_data(graph_data_input),
              builder(builder_input) {}

    const Graph& graph_data;
    caching_builder& builder;
};

} // namespace oneapi::dal::preview::subgraph_isomorphism::detail
