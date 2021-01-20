/*******************************************************************************
* Copyright 2020 Intel Corporation
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

#include "oneapi/dal/backend/linalg.hpp"

namespace oneapi::dal::test::engine {

template <typename Float>
inline double get_tolerance(double f32_tol, double f64_tol) {
    static_assert(std::is_same_v<Float, float> || std::is_same_v<Float, double>,
                  "Only single or double precision is allowed");

    if constexpr (std::is_same_v<Float, float>) {
        return f32_tol;
    }

    if constexpr (std::is_same_v<Float, double>) {
        return f64_tol;
    }
}

template <typename Reference, typename Actual>
inline double l_inf_norm(const Reference& ref, const Actual& actual) {
    const auto ref_mat = backend::linalg::matrix<double>::wrap(ref);
    const auto act_mat = backend::linalg::matrix<double>::wrap(actual);
    return backend::linalg::l_inf_norm(ref_mat, act_mat);
}

} // oneapi::dal::test::engine