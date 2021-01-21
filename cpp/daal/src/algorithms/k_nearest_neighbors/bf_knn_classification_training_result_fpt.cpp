/* file: bf_knn_classification_training_result_fpt.cpp */
/*******************************************************************************
* Copyright 2014-2021 Intel Corporation
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

#include "src/algorithms/k_nearest_neighbors/bf_knn_classification_training_result.h"

namespace daal
{
namespace algorithms
{
namespace bf_knn_classification
{
namespace training
{
template DAAL_EXPORT services::Status Result::allocate<DAAL_FPTYPE>(const daal::algorithms::Input * input, const Parameter * parameter, int method);

} // namespace training
} // namespace bf_knn_classification
} // namespace algorithms
} // namespace daal
