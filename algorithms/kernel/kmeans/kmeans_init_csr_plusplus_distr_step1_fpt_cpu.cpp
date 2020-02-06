/* file: kmeans_init_csr_plusplus_distr_step1_fpt_cpu.cpp */
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
//  Implementation of K-means plus plus initialization method for K-means algorithm
//--
*/

#include "algorithms/kernel/kmeans/kmeans_init_kernel.h"
#include "algorithms/kernel/kmeans/kmeans_init_impl.i"
#include "algorithms/kernel/kmeans/kmeans_init_container.h"

namespace daal
{
namespace algorithms
{
namespace kmeans
{
namespace init
{
namespace interface2
{
template class DistributedContainer<step1Local, DAAL_FPTYPE, plusPlusCSR, DAAL_CPU>;
}
namespace internal
{
template class KMeansInitStep1LocalKernel<plusPlusCSR, DAAL_FPTYPE, DAAL_CPU>;
} // namespace internal
} // namespace init
} // namespace kmeans
} // namespace algorithms
} // namespace daal
