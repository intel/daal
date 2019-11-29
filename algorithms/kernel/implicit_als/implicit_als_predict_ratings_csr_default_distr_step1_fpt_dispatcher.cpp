/* file: implicit_als_predict_ratings_csr_default_distr_step1_fpt_dispatcher.cpp */
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

/*
//++
//  Implementation of implicit ALS distributed prediction algorithm container.
//--
*/

#include "kernel.h"
#include "implicit_als_predict_ratings_distributed.h"

namespace daal
{
namespace algorithms
{
__DAAL_INSTANTIATE_DISPATCH_CONTAINER(implicit_als::prediction::ratings::DistributedContainer, distributed, step1Local, DAAL_FPTYPE,
                                      implicit_als::prediction::ratings::defaultDense)
}
} // namespace daal
