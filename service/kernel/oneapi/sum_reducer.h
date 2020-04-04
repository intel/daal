/* file: sum_reducer.h */
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

#ifndef __SUM_REDUCER_H__
#define __SUM_REDUCER_H__

#include "service/kernel/oneapi/math_service_types.h"
#include "services/buffer.h"
#include "service/kernel/oneapi/cl_kernels/sum_reducer.cl"
#include "oneapi/internal/types_utils.h"
#include "oneapi/internal/execution_context.h"

namespace daal
{
namespace oneapi
{
namespace internal
{
namespace math
{
class SumReducer
{
public:
    struct Result
    {
        UniversalBuffer sum;
        UniversalBuffer sumOfSquares;

        Result(ExecutionContextIface & context, uint32_t nVectors, TypeId type, services::Status * status)
            : sum(context.allocate(type, nVectors, status)), sumOfSquares(context.allocate(type, nVectors, status))
        {}
    };

public:
    static Result sum(Layout vectorsLayout, const UniversalBuffer & vectors, uint32_t nVectors, uint32_t vectorSize, services::Status * status);

private:
    SumReducer();
};

class Reducer
{
public:
    enum class BinaryOp
    {
        MIN,
        MAX
    };

    struct Result
    {
        UniversalBuffer reduce;

        Result(ExecutionContextIface & context, uint32_t nVectors, TypeId type, services::Status * status)
            : reduce(context.allocate(type, nVectors, status))
        {}
    };

public:
    static Result reduce(const BinaryOp op, Layout vectorsLayout, const UniversalBuffer & vectors, uint32_t nVectors, uint32_t vectorSize,
                         services::Status * status);

private:
    Reducer();

    static services::Status buildProgram(ClKernelFactoryIface & kernelFactory, const BinaryOp op, const TypeId & vectorType);
    static void singlepass(ExecutionContextIface & context, ClKernelFactoryIface & kernelFactory, Layout vectorsLayout,
                           const UniversalBuffer & vectors, uint32_t nVectors, uint32_t vectorSize, uint32_t workItemsPerGroup,
                           Reducer::Result & result, services::Status * status);
    static void run_step_colmajor(ExecutionContextIface & context, ClKernelFactoryIface & kernelFactory, const UniversalBuffer & vectors,
                                  uint32_t nVectors, uint32_t vectorSize, uint32_t numWorkItems, uint32_t numWorkGroups, Reducer::Result & stepResult,
                                  services::Status * status);
    static void run_final_step_rowmajor(ExecutionContextIface & context, ClKernelFactoryIface & kernelFactory, Reducer::Result & stepResult,
                                        uint32_t nVectors, uint32_t vectorSize, uint32_t workItemsPerGroup, Reducer::Result & result,
                                        services::Status * status);
};

} // namespace math
} // namespace internal
} // namespace oneapi
} // namespace daal

#endif
