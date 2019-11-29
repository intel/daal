/* file: mt2203_dense_default_batch_fpt_cpu.cpp */
/*******************************************************************************
* Copyright 2014-2019 Intel Corporation
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
//  Implementation of mt2203 calculation functions.
//--

#include "mt2203_batch_container.h"
#include "mt2203_kernel.h"
#include "mt2203_impl.i"

namespace daal
{
namespace algorithms
{
namespace engines
{
namespace mt2203
{
namespace interface1
{
template class BatchContainer<DAAL_FPTYPE, defaultDense, DAAL_CPU>;
} // namespace interface1

namespace internal
{
template class Mt2203Kernel<DAAL_FPTYPE, defaultDense, DAAL_CPU>;
} // namespace internal

} // namespace mt2203
} // namespace engines
} // namespace algorithms
} // namespace daal
