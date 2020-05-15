/* file: maximum_pooling1d_layer_backward_impl.i */
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

/*
//++
//  Implementation of backward pooling layer
//--
*/

#ifndef __MAXIMUM_POOLING1D_LAYER_BACKWARD_IMPL_I__
#define __MAXIMUM_POOLING1D_LAYER_BACKWARD_IMPL_I__

#include "externals/service_memory.h"
#include "externals/service_blas.h"
#include "service/kernel/data_management/service_tensor.h"
#include "service/kernel/data_management/service_numeric_table.h"

#include "algorithms/kernel/neural_networks/layers/pooling1d_layer/pooling1d_layer_impl.i"

using namespace daal::services;
using namespace daal::internal;

namespace daal
{
namespace algorithms
{
namespace neural_networks
{
namespace layers
{
namespace maximum_pooling1d
{
namespace backward
{
namespace internal
{
template <typename algorithmFPType, Method method, CpuType cpu>
services::Status PoolingKernel<algorithmFPType, method, cpu>::compute(const Tensor & inputGradTensor, const Tensor & selectedPosTensor,
                                                                      Tensor & gradTensor, const maximum_pooling1d::Parameter & parameter)
{
    const algorithmFPType zero = 0.0;

    const Collection<size_t> & inputDims = inputGradTensor.getDimensions();
    const Collection<size_t> & gradDims  = gradTensor.getDimensions();

    ReadSubtensor<algorithmFPType, cpu> inputGradSubtensor(const_cast<Tensor &>(inputGradTensor), 0, 0, 0, inputDims[0]);
    DAAL_CHECK_BLOCK_STATUS(inputGradSubtensor);
    const algorithmFPType * inputGrad = inputGradSubtensor.get();

    ReadSubtensor<int, cpu> selectedPosSubtensor(const_cast<Tensor &>(selectedPosTensor), 0, 0, 0, inputDims[0]);
    DAAL_CHECK_BLOCK_STATUS(selectedPosSubtensor);
    const int * selectedPos = selectedPosSubtensor.get();

    WriteOnlySubtensor<algorithmFPType, cpu> gradSubtensor(gradTensor, 0, 0, 0, gradDims[0]);
    DAAL_CHECK_BLOCK_STATUS(gradSubtensor);
    algorithmFPType * grad = gradSubtensor.get();

    const size_t gradSize = gradTensor.getSize();
    daal::services::internal::service_memset<algorithmFPType, cpu>(grad, zero, gradSize);

    pooling1d::internal::Parameter par(parameter.index.size[0], parameter.padding.size[0], parameter.stride.size[0], parameter.kernelSize.size[0],
                                       gradTensor, gradDims, inputDims);

    for (DAAL_INT i = 0; i < par.offsetBefore; i++)
    {
        /*
         * Loop by the first kernel dimension
         * f - index of the left upper corner of the kernel
         * fo - index of the output value
         */
        for (DAAL_INT f = -par.padding, fo = 0; fo < par.firstOutSize; f += par.stride, fo++)
        {
            for (DAAL_INT j = 0; j < par.offsetAfter; j++)
            {
                /*
                 * Input value index
                 */
                const DAAL_INT inputIndex = j + par.offsetAfter * (fo + par.firstOutSize * i);
                const DAAL_INT fi         = f + selectedPos[inputIndex];
                const bool paddingFlag    = (fi < 0) || (fi >= par.firstSize);
                if (!paddingFlag && selectedPos[inputIndex] >= 0)
                {
                    DAAL_INT gradIndex = j + par.offsetAfter * (fi + par.firstSize * i);
                    grad[gradIndex] += inputGrad[inputIndex];
                }
            }
        }
    }
    return Status();
}

} // namespace internal
} // namespace backward
} // namespace maximum_pooling1d
} // namespace layers
} // namespace neural_networks
} // namespace algorithms
} // namespace daal

#endif
