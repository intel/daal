/* file: kmeans_init_dense_plusplus_distr_step3_fpt_cpu.cpp */
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
//  Implementation of k-means plus plus initialization method for K-means algorithm
//--
*/

#include "kmeans_init_kernel.h"
#include "kmeans_init_container.h"
#include "kmeans_plusplus_init_distr_impl.i"

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
template class DistributedContainer<step3Master, DAAL_FPTYPE, plusPlusDense, DAAL_CPU>;
}
namespace internal
{
template class KMeansInitStep3MasterKernel<plusPlusDense, DAAL_FPTYPE, DAAL_CPU>;
} // namespace internal
} // namespace init
} // namespace kmeans
} // namespace algorithms
} // namespace daal
