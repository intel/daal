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

#define DAAL_SYCL_INTERFACE

#include "oneapi/dal/algo/knn/backend/gpu/train_kernel.hpp"
#include "oneapi/dal/backend/dispatcher_dpc.hpp"
#include "oneapi/dal/backend/interop/common_dpc.hpp"
#include "oneapi/dal/backend/interop/error_converter.hpp"

namespace oneapi::dal::knn::backend {

using dal::backend::context_gpu;

template <typename Float, typename Task>
struct train_kernel_gpu<Float, method::kd_tree, Task> {
    train_result<Task> operator()(const context_gpu& ctx,
                                  const descriptor_base<Task>& desc,
                                  const train_input<Task>& input) const {
        throw unimplemented("k-NN k-d tree method is not implemented for GPU");
        return train_result<Task>();
    }
};

template struct train_kernel_gpu<float, method::kd_tree, task::classification>;
template struct train_kernel_gpu<double, method::kd_tree, task::classification>;

} // namespace oneapi::dal::knn::backend
