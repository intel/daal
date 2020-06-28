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

#include "oneapi/dal/algo/linear_kernel/backend/cpu/compute_kernel.hpp"
#include "oneapi/dal/algo/linear_kernel/backend/gpu/compute_kernel.hpp"
#include "oneapi/dal/algo/linear_kernel/detail/compute_ops.hpp"
#include "oneapi/dal/backend/dispatcher_dp.hpp"

namespace oneapi::dal::linear_kernel::detail {

template <typename Float, typename Method>
struct compute_ops_dispatcher<data_parallel_execution_context, Float, Method> {
    compute_result operator()(const data_parallel_execution_context& ctx,
                              const descriptor_base& params,
                              const compute_input& input) const {
        using kernel_dispatcher_t =
            dal::backend::kernel_dispatcher<backend::compute_kernel_cpu<Float, Method>,
                                            backend::compute_kernel_gpu<Float, Method>>;
        return kernel_dispatcher_t{}(ctx, params, input);
    }
};

#define INSTANTIATE(F, M) \
    template struct compute_ops_dispatcher<data_parallel_execution_context, F, M>;

INSTANTIATE(float, method::default_dense)
INSTANTIATE(float, method::fast_csr)
INSTANTIATE(double, method::default_dense)
INSTANTIATE(double, method::fast_csr)

} // namespace oneapi::dal::linear_kernel::detail
