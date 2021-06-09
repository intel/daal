/*******************************************************************************
* Copyright 2021 Intel Corporation
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

#include <pybind11/pybind11.h>

#include "oneapi/dal/algo/linear_kernel.hpp"
#include "oneapi/dal/algo/polynomial_kernel.hpp"
#include "oneapi/dal/algo/rbf_kernel.hpp"

namespace oneapi::dal {

template <typename Kernel>
auto get_kernel_descriptor(const pybind11::dict& params) {
    using float_t = typename Kernel::float_t;
    using method_t = typename Kernel::method_t;
    using task_t = typename Kernel::task_t;
    using linear_desc_t = linear_kernel::descriptor<float_t, method_t, task_t>;
    using polynomial_desc_t = polynomial_kernel::descriptor<float_t, method_t, task_t>;
    using rbf_desc_t = rbf_kernel::descriptor<float_t, method_t, task_t>;

    auto kernel = Kernel{};
    if constexpr (std::is_same_v<Kernel, linear_desc_t>) {
        kernel.set_scale(params["scale"].cast<double>()).set_shift(params["shift"].cast<double>());
    }
    if constexpr (std::is_same_v<Kernel, polynomial_desc_t>) {
        kernel.set_scale(params["scale"].cast<double>())
            .set_shift(params["shift"].cast<double>())
            .set_degree(params["degree"].cast<std::int64_t>());
    }
    if constexpr (std::is_same_v<Kernel, rbf_desc_t>) {
        kernel.set_sigma(params["sigma"].cast<double>());
    }
    return kernel;
}

template <typename Result>
void init_kernel_result(pybind11::module_& m) {
    pybind11::class_<Result>(m, "result")
        .def(pybind11::init())
        .def_property("values", &Result::get_values, &Result::set_values);
}

} // namespace oneapi::dal
