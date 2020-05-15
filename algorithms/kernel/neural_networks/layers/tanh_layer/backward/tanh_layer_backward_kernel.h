/* file: tanh_layer_backward_kernel.h */
/*******************************************************************************
* Copyright 2014-2020 Intel Corporation
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

//++
//  Declaration of template function that calculate hyperbolic tangent functions.
//--

#ifndef __TANH_LAYER_BACKWARD_KERNEL_H__
#define __TANH_LAYER_BACKWARD_KERNEL_H__

#include "algorithms/neural_networks/layers/tanh/tanh_layer.h"
#include "algorithms/neural_networks/layers/tanh/tanh_layer_types.h"
#include "algorithms/kernel/kernel.h"
#include "externals/service_math.h"
#include "data_management/data/numeric_table.h"
#include "algorithms/kernel/neural_networks/layers/layers_threading.h"

using namespace daal::data_management;
using namespace daal::services;
using namespace daal::algorithms::neural_networks::layers::internal;

namespace daal
{
namespace algorithms
{
namespace neural_networks
{
namespace layers
{
namespace tanh
{
namespace backward
{
namespace internal
{
/**
 *  \brief Kernel for hyperbolic tangent function calculation
 */
template <typename algorithmFPType, Method method, CpuType cpu>
class TanhKernel : public Kernel
{
public:
    services::Status compute(const Tensor & inputTensor, const Tensor & forwardOutputTensor, Tensor & resultTensor);
};

} // namespace internal
} // namespace backward
} // namespace tanh
} // namespace layers
} // namespace neural_networks
} // namespace algorithms
} // namespace daal

#endif
