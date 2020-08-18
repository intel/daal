/* file: svm_regression_train_thunder_batch_fpt_dispatcher.cpp */
/*******************************************************************************
* Copyright 2020 Intel Corporation
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
//  Implementation of SVM training algorithm container.
//--
*/

#include "src/algorithms/svm/regression/svm_regression_train_batch_container.h"

namespace daal
{
namespace algorithms
{
__DAAL_INSTANTIATE_DISPATCH_CONTAINER_SYCL_SAFE(svm::regression::training::BatchContainer, batch, DAAL_FPTYPE, svm::training::thunder)

namespace svm
{
namespace regression
{
namespace training
{
namespace interface3
{
using BatchType = Batch<DAAL_FPTYPE, svm::training::thunder>;

template <>
BatchType::Batch()
{
    _par = new ParameterType();
    initialize();
}

template <>
BatchType::Batch(const BatchType & other) : input(other.input)
{
    _par = new ParameterType(other.parameter());
    initialize();
}

} // namespace interface3
} // namespace training
} // namespace regression
} // namespace svm

} // namespace algorithms
} // namespace daal
