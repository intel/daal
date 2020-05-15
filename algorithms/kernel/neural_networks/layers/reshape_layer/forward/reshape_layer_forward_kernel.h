/* file: reshape_layer_forward_kernel.h */
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
//  Implementation of reshape layer.
//--

#ifndef __RESHAPE_LAYER_FORWARD_KERNEL_H__
#define __RESHAPE_LAYER_FORWARD_KERNEL_H__

#include "algorithms/neural_networks/layers/reshape/reshape_layer.h"
#include "algorithms/neural_networks/layers/reshape/reshape_layer_types.h"
#include "algorithms/kernel/kernel.h"
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
namespace reshape
{
namespace forward
{
namespace internal
{
/**
 *  \brief Kernel for reshape calculation
 */
template <typename algorithmFPType, Method method, CpuType cpu>
class ReshapeKernel : public Kernel
{
public:
    services::Status compute(const Tensor & inputTensor, Tensor & resultTensor);
};

} // namespace internal
} // namespace forward
} // namespace reshape
} // namespace layers
} // namespace neural_networks
} // namespace algorithms
} // namespace daal

#endif
