/* file: logitboost_train_batch_container_v1.h */
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
//  Implementation of Logit Boost container.
//--
*/

#ifndef __LOGITBOOST_TRAIN_BATCH_CONTAINER_V1_H__
#define __LOGITBOOST_TRAIN_BATCH_CONTAINER_V1_H__

#include "algorithms/boosting/logitboost_training_batch.h"
#include "algorithms/kernel/logitboost/inner/logitboost_train_friedman_kernel_v1.h"
#include "algorithms/kernel/kernel.h"

namespace daal
{
namespace algorithms
{
namespace logitboost
{
namespace training
{
namespace interface1
{
template <typename algorithmFPType, Method method, CpuType cpu>
BatchContainer<algorithmFPType, method, cpu>::BatchContainer(daal::services::Environment::env * daalEnv)
{
    __DAAL_INITIALIZE_KERNELS(internal::I1LogitBoostTrainKernel, method, algorithmFPType);
}

template <typename algorithmFPType, Method method, CpuType cpu>
BatchContainer<algorithmFPType, method, cpu>::~BatchContainer()
{
    __DAAL_DEINITIALIZE_KERNELS();
}

template <typename algorithmFPType, Method method, CpuType cpu>
services::Status BatchContainer<algorithmFPType, method, cpu>::compute()
{
    classifier::training::interface1::Input * input   = static_cast<classifier::training::interface1::Input *>(_in);
    classifier::training::interface1::Result * result = static_cast<classifier::training::interface1::Result *>(_res);

    size_t na = input->size();

    NumericTablePtr a[2];
    a[0]                                    = services::staticPointerCast<NumericTable>(input->get(classifier::training::data));
    a[1]                                    = services::staticPointerCast<NumericTable>(input->get(classifier::training::labels));
    logitboost::interface1::Model * r       = static_cast<logitboost::interface1::Model *>(result->get(classifier::training::model).get());
    logitboost::interface1::Parameter * par = static_cast<logitboost::interface1::Parameter *>(_par);

    daal::services::Environment::env & env = *_env;
    __DAAL_CALL_KERNEL(env, internal::I1LogitBoostTrainKernel, __DAAL_KERNEL_ARGUMENTS(method, algorithmFPType), compute, na, a, r, par);
}
} // namespace interface1
} // namespace training
} // namespace logitboost
} // namespace algorithms
} // namespace daal

#endif // __LOGITBOOST_TRAIN_BATCH_CONTAINER_V1_H__
