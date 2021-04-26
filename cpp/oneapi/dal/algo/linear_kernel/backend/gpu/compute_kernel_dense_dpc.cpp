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

#include "oneapi/dal/algo/linear_kernel/backend/gpu/compute_kernel.hpp"
#include "oneapi/dal/backend/primitives/blas/gemm.hpp"
#include "oneapi/dal/table/row_accessor.hpp"

namespace oneapi::dal::linear_kernel::backend {

using dal::backend::context_gpu;
using input_t = compute_input<task::compute>;
using result_t = compute_result<task::compute>;
using descriptor_t = detail::descriptor_base<task::compute>;

namespace pr = dal::backend::primitives;

template <typename Float>
static result_t compute(const context_gpu& ctx, const descriptor_t& desc, const input_t& input) {
    const auto x = input.get_x();
    const auto y = input.get_y();

    auto& queue = ctx.get_queue();

    const std::int64_t row_count_x = x.get_row_count();
    const std::int64_t row_count_y = y.get_row_count();

    ONEDAL_ASSERT(col_count_x == col_count_y);
    dal::detail::check_mul_overflow(row_count_x, row_count_y);

    const Float scale = desc.get_scale();
    const Float shift = desc.get_shift();

    const auto ndarray_x =
        pr::flatten_table<Float, row_accessor>(queue, x, sycl::usm::alloc::device);

    const auto ndarray_y =
        pr::flatten_table<Float, row_accessor>(queue, y, sycl::usm::alloc::device);

    auto ndarray_res =
        pr::ndarray<Float, 2>::empty(queue, { row_count_x, row_count_y }, sycl::usm::alloc::device);

    sycl::event fill_res_event;
    if (shift != 0.0) {
        fill_res_event = ndarray_res.fill(queue, Float(1));
    }

    auto gemm_event =
        gemm(queue, ndarray_x, ndarray_y.t(), ndarray_res, scale, shift, { fill_res_event });

    return result_t{}.set_values(
        homogen_table::wrap(ndarray_res.flatten(queue), row_count_x, row_count_y, { gemm_event }));
}

template <typename Float>
struct compute_kernel_gpu<Float, method::dense, task::compute> {
    result_t operator()(const context_gpu& ctx,
                        const descriptor_t& desc,
                        const input_t& input) const {
        return compute<Float>(ctx, desc, input);
    }
};

template struct compute_kernel_gpu<float, method::dense, task::compute>;
template struct compute_kernel_gpu<double, method::dense, task::compute>;

} // namespace oneapi::dal::linear_kernel::backend
