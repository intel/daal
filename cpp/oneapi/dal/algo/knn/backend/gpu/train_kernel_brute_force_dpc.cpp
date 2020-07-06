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
#include "src/algorithms/k_nearest_neighbors/oneapi/bf_knn_classification_train_kernel_ucapi.h"
#include "algorithms/engines/mcg59/mcg59.h"
#include "data_management/data/numeric_table.h"
#include "src/algorithms/k_nearest_neighbors/oneapi/bf_knn_classification_model_ucapi_impl.h"

#include "oneapi/dal/algo/knn/backend/gpu/train_kernel.hpp"
#include "oneapi/dal/algo/knn/backend/model_interop.hpp"
#include "oneapi/dal/algo/knn/detail/model_impl.hpp"
#include "oneapi/dal/backend/dispatcher_dpc.hpp"
#include "oneapi/dal/backend/interop/table_conversion.hpp"

namespace oneapi::dal::knn::backend {

using dal::backend::context_gpu;
using namespace daal::data_management;

namespace daal_knn = daal::algorithms::bf_knn_classification;
namespace interop  = dal::backend::interop;

template <typename Float>
using daal_knn_brute_force_kernel_t = daal_knn::training::internal::
    KNNClassificationTrainKernelUCAPI<Float>;
using daal_interop_model_t = detail::model_impl::interop_model;

template <typename Float>
static train_result call_daal_kernel(const context_gpu& ctx,
                                     const descriptor_base& desc,
                                     const table& data,
                                     const table& labels) {
    const std::int64_t row_count    = data.get_row_count();
    const std::int64_t column_count = data.get_column_count();

    auto arr_data   = row_accessor<const Float>{ data }.pull();
    auto arr_labels = row_accessor<const Float>{ labels }.pull();
    // TODO: read-only access performed with deep copy of data since daal numeric tables are mutable.
    // Need to create special immutable homogen table on daal interop side

    // TODO: data is table, not a homogen_table. Think better about accessor - is it enough to have just a row_accessor?
    const auto daal_data =
        interop::convert_to_daal_homogen_table(arr_data, row_count, column_count);
    const auto daal_labels = interop::convert_to_daal_homogen_table(arr_labels, row_count, 1);

    daal_knn::Parameter daal_parameter(
        desc.get_class_count(),
        desc.get_neighbor_count(),
        desc.get_data_use_in_model() ? daal_knn::doUse : daal_knn::doNotUse);

    daal::algorithms::classifier::ModelPtr model_ptr(new daal_knn::Model(column_count));
    //if(!model_ptr)
    //    bad alloc
    auto knn_model  = static_cast<daal_knn::Model*>(model_ptr.get());

    knn_model->impl()->setData<Float>(daal_data, desc.get_data_use_in_model());
    knn_model->impl()->setLabels<Float>(daal_labels, desc.get_data_use_in_model());

    auto queue = ctx.get_queue();
    daal::services::SyclExecutionContext daal_ctx(queue);
    daal::services::Environment::getInstance()->setDefaultExecutionContext(daal_ctx);

    daal_knn_brute_force_kernel_t<Float>().compute(
        daal_data.get(),
        daal_labels.get(),
        knn_model,
        daal_parameter,
        *(daal::algorithms::engines::mcg59::Batch<>::create()));
    daal::services::Environment::getInstance()->setDefaultExecutionContext(daal::services::CpuExecutionContext());
    auto interop          = new daal_interop_model_t(model_ptr);
    const auto model_impl = std::make_shared<detail::model_impl>(interop);
    return train_result().set_model(dal::detail::pimpl_accessor::make<model>(model_impl));
}

template <typename Float>
static train_result train(const context_gpu& ctx,
                          const descriptor_base& desc,
                          const train_input& input) {
    return call_daal_kernel<Float>(ctx, desc, input.get_data(), input.get_labels());
}

template <typename Float>
struct train_kernel_gpu<Float, method::brute_force> {
    train_result operator()(const context_gpu& ctx,
                            const descriptor_base& desc,
                            const train_input& input) const {
        return train<Float>(ctx, desc, input);
    }
};

template struct train_kernel_gpu<float, method::brute_force>;
template struct train_kernel_gpu<double, method::brute_force>;

} // namespace oneapi::dal::knn::backend
