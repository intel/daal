/* file: concat_layer_forward_kernel.h */
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
//  Declaration of template function that calculate concats.
//--

#ifndef __CONCAT_LAYER_FORWARD_KERNEL_H__
#define __CONCAT_LAYER_FORWARD_KERNEL_H__

#include "algorithms/neural_networks/layers/concat/concat_layer.h"
#include "algorithms/neural_networks/layers/concat/concat_layer_types.h"
#include "algorithms/kernel/kernel.h"
#include "externals/service_dnn.h"
#include "algorithms/kernel/service_dnn_internal.h"
#include "algorithms/kernel/neural_networks/layers/layers_threading.h"

using namespace daal::data_management;
using namespace daal::services;

namespace daal
{
namespace algorithms
{
namespace neural_networks
{
namespace layers
{
namespace concat
{
namespace forward
{
namespace internal
{
/**
 *  \brief Kernel for concat calculation
 */
template <typename algorithmFPType, Method method, CpuType cpu>
class ConcatKernel : public Kernel
{
public:
    services::Status compute(size_t nInputs, Tensor * inputTensors[], const concat::Parameter * parameter, Tensor * resultTensor);

    ~ConcatKernel()
    {
        if (concatPrim)
        {
            dnn::xDelete(concatPrim);
        }
        if (inputLayouts)
        {
            delete[] inputLayouts;
        }
    }

private:
    typedef daal::internal::Dnn<algorithmFPType, cpu> dnn;

    const size_t _nRowsInBlock = 5000;

    dnnPrimitive_t concatPrim  = NULL;
    dnnLayout_t * inputLayouts = NULL;
};
} // namespace internal
} // namespace forward
} // namespace concat
} // namespace layers
} // namespace neural_networks
} // namespace algorithms
} // namespace daal

#endif
